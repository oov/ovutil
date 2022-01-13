#include "ovutil/win32.h"

#include "ovutil/str.h"

#include <shlwapi.h>

static HINSTANCE g_hinstance = NULL;

void set_hinstance(HINSTANCE const h) { g_hinstance = h; }

HINSTANCE get_hinstance(void) { return g_hinstance; }

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  if __has_warning("-Wpadded")
#    pragma GCC diagnostic ignored "-Wpadded"
#  endif
#endif
struct disable_family_windows_data {
  HWND *ptr;
  size_t len;
  size_t cap;

  DWORD pid;
  HWND exclude;
};
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

static WINBOOL CALLBACK disable_family_windows_callback(HWND const window, LPARAM const lparam) {
  struct disable_family_windows_data *const d = (void *)lparam;
  if (!IsWindowVisible(window) || !IsWindowEnabled(window) || d->exclude == window) {
    return TRUE;
  }
  DWORD pid = 0;
  GetWindowThreadProcessId(window, &pid);
  if (pid != d->pid) {
    return TRUE;
  }
  error err = apush(d, window);
  if (efailed(err)) {
    efree(&err);
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
  }

  return TRUE;
}

error disable_family_windows(HWND const exclude, HWND **const disabled_windows) {
  if (!disabled_windows) {
    return errg(err_null_pointer);
  }
  error err = eok();
  struct disable_family_windows_data d = {
      .pid = GetCurrentProcessId(),
      .exclude = exclude,
  };
  if (!EnumWindows(disable_family_windows_callback, (LPARAM)&d)) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto failed;
  }
  err = apush(&d, INVALID_HANDLE_VALUE);
  if (efailed(err)) {
    efree(&err);
    err = errhr(HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY));
    goto failed;
  }

  HWND const *h = d.ptr;
  while (*h != INVALID_HANDLE_VALUE) {
    EnableWindow(*h++, FALSE);
  }
  *disabled_windows = d.ptr;
  return eok();

failed:
  ereport(afree(&d));
  return err;
}

void restore_disabled_family_windows(HWND *const disabled_windows) {
  if (!disabled_windows) {
    return;
  }
  struct disable_family_windows_data d = {
      .ptr = disabled_windows,
      .len = 1,
      .cap = 1,
  };
  HWND *h = disabled_windows;
  while (*h != INVALID_HANDLE_VALUE) {
    EnableWindow(*h++, TRUE);
    d.cap = ++d.len;
  }
  ereport(afree(&d));
}

int message_box(HWND const window, wchar_t const *const msg, wchar_t const *const title, UINT const flags) {
  HWND *w = NULL;
  error err = disable_family_windows(window, &w);
  if (efailed(err)) {
    efree(&err);
  }
  int const r = MessageBoxW(window, msg, title, flags);
  restore_disabled_family_windows(w);
  return r;
}

error set_client_size(HWND const window, LONG const width, LONG const height) {
  RECT wr = {0};
  if (!GetWindowRect(window, &wr)) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  RECT cr = {0};
  if (!GetClientRect(window, &cr)) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  if (!SetWindowPos(window,
                    NULL,
                    0,
                    0,
                    (wr.right - wr.left) - (cr.right - cr.left) + width,
                    (wr.bottom - wr.top) - (cr.bottom - cr.top) + height,
                    SWP_NOMOVE | SWP_NOZORDER)) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  return eok();
}

error get_window_text(HWND const window, struct wstr *const dest) {
  if (!window) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  struct wstr tmp = {0};
  error err = eok();
  SetLastError(0);
  int slen = GetWindowTextLengthW(window);
  if (slen == 0) {
    HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
    if (FAILED(hr)) {
      err = errhr(hr);
      goto cleanup;
    }
  }
  err = sgrow(&tmp, (size_t)(slen + 1));
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

  int const written = GetWindowTextW(window, tmp.ptr, slen + 1);
  if (written == 0) {
    HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
    if (FAILED(hr)) {
      err = errhr(hr);
      goto cleanup;
    }
  }
  tmp.len = (size_t)slen;

  err = scpy(dest, tmp.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  ereport(sfree(&tmp));
  return err;
}

error get_module_file_name(HINSTANCE const hinst, struct wstr *const dest) {
  if (!dest) {
    return errg(err_null_pointer);
  }

  error err = eok();
  struct wstr tmp = {0};
  DWORD n = 0;
  DWORD r = 0;
  do {
    err = sgrow(&tmp, n += MAX_PATH);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    r = GetModuleFileNameW(hinst, tmp.ptr, n);
  } while (n == r);
  if (r == 0) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  tmp.len = r;
  err = scpy(dest, tmp.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  ereport(sfree(&tmp));
  return err;
}

error get_temp_dir(struct wstr *const dest) {
  if (!dest) {
    return errg(err_null_pointer);
  }

  struct wstr tmp = {0};
  struct wstr tempdir = {0};
  error err = sgrow(&tmp, MAX_PATH + 1);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

  if (!GetTempPathW(MAX_PATH + 1, tmp.ptr)) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  tmp.len = wcslen(tmp.ptr);

  err = get_long_path_name(&tmp, &tempdir);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

  err = scpy(dest, tempdir.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  ereport(sfree(&tempdir));
  ereport(sfree(&tmp));
  return err;
}

NODISCARD static error get_file_attributes(struct wstr const *const path, DWORD *const attr) {
  if (!path) {
    return errg(err_invalid_arugment);
  }
  if (!attr) {
    return errg(err_null_pointer);
  }

  DWORD const r = GetFileAttributesW(path->ptr);
  if (r == INVALID_FILE_ATTRIBUTES) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  *attr = r;
  return eok();
}

error file_exists(struct wstr const *const path, bool *const exists) {
  if (!path) {
    return errg(err_invalid_arugment);
  }
  if (!exists) {
    return errg(err_null_pointer);
  }

  DWORD attr = 0;
  error err = get_file_attributes(path, &attr);
  if (efailed(err)) {
    if (eis_hr(err, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) ||
        eis_hr(err, HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))) {
      efree(&err);
      *exists = false;
      return eok();
    }
    err = ethru(err);
    return err;
  }
  *exists = true;
  return eok();
}

error get_long_path_name(struct wstr const *const src, struct wstr *const dest) {
  if (!src) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }
  struct wstr tmp = {0};
  error err = eok();
  DWORD len = GetLongPathNameW(src->ptr, NULL, 0);
  if (len == 0) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  err = sgrow(&tmp, len);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  len = GetLongPathNameW(src->ptr, tmp.ptr, len);
  if (len == 0) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  tmp.len = len;
  err = scpy(dest, tmp.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  ereport(sfree(&tmp));
  return err;
}

NODISCARD static error path_relative_path_to(struct wstr const *const path_from,
                                             DWORD const attr_from,
                                             struct wstr const *const path_to,
                                             DWORD const attr_to,
                                             struct wstr *const dest) {
  struct wstr tmp = {0};
  error err = sgrow(&tmp, MAX_PATH);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  if (PathRelativePathToW(tmp.ptr, path_from->ptr, attr_from, path_to->ptr, attr_to)) {
    tmp.len = wcslen(tmp.ptr);
  }
  err = scpy(dest, tmp.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  ereport(sfree(&tmp));
  return err;
}

error file_contains(struct wstr const *const dir, struct wstr const *const file, bool *const contains) {
  struct wstr tmp = {0};
  error err = path_relative_path_to(dir, FILE_ATTRIBUTE_DIRECTORY, file, 0, &tmp);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  if (!tmp.len) {
    *contains = false;
    goto cleanup;
  }
  if (wcsncmp(tmp.ptr, L"..\\", 3) == 0 || wcsncmp(tmp.ptr, L"../", 3) == 0) {
    *contains = false;
    goto cleanup;
  }

  *contains = true;

cleanup:
  ereport(sfree(&tmp));
  return err;
}

NODISCARD static error get_file_information(struct wstr const *const path, BY_HANDLE_FILE_INFORMATION *const bhfi) {
  if (!path) {
    return errg(err_invalid_arugment);
  }
  if (!bhfi) {
    return errg(err_null_pointer);
  }
  DWORD attr = 0;
  error err = get_file_attributes(path, &attr);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  DWORD const fa = (attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY ? FILE_FLAG_BACKUP_SEMANTICS : 0;
  HANDLE const h =
      CreateFileW(path->ptr, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, fa, NULL);
  if (h == INVALID_HANDLE_VALUE) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  BY_HANDLE_FILE_INFORMATION fi = {0};
  if (!GetFileInformationByHandle(h, &fi)) {
    HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
    CloseHandle(h);
    return errhr(hr);
  }
  CloseHandle(h);
  *bhfi = fi;
  return eok();
}

error is_same_file(struct wstr const *const file1, struct wstr const *const file2, bool *const same) {
  if (!file1 || !file2) {
    return errg(err_invalid_arugment);
  }
  if (!same) {
    return errg(err_null_pointer);
  }
  BY_HANDLE_FILE_INFORMATION fi1 = {0};
  BY_HANDLE_FILE_INFORMATION fi2 = {0};
  error err = get_file_information(file1, &fi1);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  err = get_file_information(file2, &fi2);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  *same = fi1.dwVolumeSerialNumber == fi2.dwVolumeSerialNumber && fi1.nFileIndexLow == fi2.nFileIndexLow &&
          fi1.nFileIndexHigh == fi2.nFileIndexHigh;
  return eok();
}

error is_same_dir(struct wstr const *const dir1, struct wstr const *const dir2, bool *const same) {
  return is_same_file(dir1, dir2, same);
}

error create_unique_temp_file(wchar_t const *const base_filename,
                              wchar_t const *const ext,
                              void *const data,
                              size_t const datalen,
                              struct wstr *const dest) {
  if (!base_filename || !ext) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  struct wstr tmp = {0};
  error err = get_temp_dir(&tmp);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = include_trailing_path_delimiter(&tmp);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = scat(&tmp, base_filename);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = create_unique_file(
      tmp.ptr, ext, FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, data, datalen, dest);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  ereport(sfree(&tmp));
  return err;
}

NODISCARD static error write_file(HANDLE const h, void *const p, size_t const bytes, size_t *const written) {
  if (!h || !p) {
    return errg(err_invalid_arugment);
  }
  size_t written_total = 0;
  char *ptr = p;
  DWORD sz = 0, ret = 0;
  error err = eok();
  while (written_total < bytes) {
    if (sizeof(DWORD) < sizeof(size_t)) {
      sz = (bytes - written_total > UINT32_MAX) ? UINT32_MAX : (DWORD)(bytes - written_total);
    } else {
      sz = (DWORD)(bytes - written_total);
    }
    if (!WriteFile(h, ptr + written_total, sz, &ret, NULL)) {
      err = errhr(HRESULT_FROM_WIN32(GetLastError()));
      goto cleanup;
    }
    written_total += (size_t)ret;
  }
cleanup:
  if (written) {
    *written = written_total;
  }
  return err;
}

error create_unique_file(wchar_t const *const base_fullpath,
                         wchar_t const *const ext,
                         DWORD const file_attributes,
                         void *const data,
                         size_t const datalen,
                         struct wstr *const dest) {
  if (!base_fullpath || !ext) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  struct wstr tmp = {0};
  error err = scpy(&tmp, base_fullpath);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

  size_t const base_fullpath_len = tmp.len;
  uint32_t hash = ovbase_splitmix32_next(GetTickCount() + GetCurrentProcessId() + GetCurrentThreadId());
  wchar_t numstr[16] = {0};
  for (int i = 0; i < 9; ++i) {
    err = scatm(&tmp, numstr, ext);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    HANDLE file =
        CreateFileW(tmp.ptr, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, file_attributes, NULL);
    if (file == INVALID_HANDLE_VALUE) {
      HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
      if (hr == HRESULT_FROM_WIN32(ERROR_FILE_EXISTS)) {
        tmp.ptr[base_fullpath_len] = L'\0';
        wsprintfW(numstr, L".%d", ovbase_splitmix32(hash) & 0xffffff);
        hash = ovbase_splitmix32_next(hash);
        continue;
      }
      err = errhr(hr);
      goto cleanup;
    }
    if (data != NULL && datalen > 0) {
      size_t written = 0;
      err = write_file(file, data, datalen, &written);
      if (efailed(err)) {
        err = ethru(err);
        CloseHandle(file);
        DeleteFileW(tmp.ptr);
        goto cleanup;
      }
      if (written != datalen) {
        err = errg(err_fail);
        CloseHandle(file);
        DeleteFileW(tmp.ptr);
        goto cleanup;
      }
    }
    CloseHandle(file);

    err = scpy(dest, tmp.ptr);
    if (efailed(err)) {
      DeleteFileW(tmp.ptr);
      err = ethru(err);
      goto cleanup;
    }
    goto cleanup;
  }

  err = errhr(HRESULT_FROM_WIN32(ERROR_FILE_EXISTS));

cleanup:
  ereport(sfree(&tmp));
  return err;
}

error delete_file(struct wstr const *const path) {
  if (!DeleteFileW(path->ptr)) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  return eok();
}

error from_cp(UINT const code_page, struct str const *const src, struct wstr *const dest) {
  if (!src) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  struct wstr tmp = {0};
  error err = eok();

  if (src->len == 0) {
    err = scpy(dest, L"");
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    return eok();
  }

  int const dlen = MultiByteToWideChar(code_page, 0, src->ptr, (int)src->len, NULL, 0);
  if (!dlen) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  err = sgrow(&tmp, (size_t)(dlen + 1));
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

  if (!MultiByteToWideChar(code_page, 0, src->ptr, (int)src->len, tmp.ptr, dlen)) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  tmp.ptr[dlen] = L'\0';
  tmp.len = (size_t)dlen;

  err = scpy(dest, tmp.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  ereport(sfree(&tmp));
  return err;
}

error to_cp(UINT const code_page, struct wstr const *const src, struct str *const dest) {
  if (!src) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  struct str tmp = {0};
  error err = eok();

  if (src->len == 0) {
    err = scpy(dest, "");
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    return eok();
  }

  int const dlen = WideCharToMultiByte(code_page, 0, src->ptr, (int)src->len, NULL, 0, NULL, NULL);
  if (!dlen) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  err = sgrow(&tmp, (size_t)(dlen + 1));
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

  if (!WideCharToMultiByte(code_page, 0, src->ptr, (int)src->len, tmp.ptr, dlen, NULL, NULL)) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  tmp.ptr[dlen] = '\0';
  tmp.len = (size_t)dlen;

  err = scpy(dest, tmp.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  ereport(sfree(&tmp));
  return err;
}

error from_mbcs(struct str const *const src, struct wstr *const dest) { return from_cp(CP_ACP, src, dest); }

error from_utf8(struct str const *const src, struct wstr *const dest) { return from_cp(CP_UTF8, src, dest); }

error to_mbcs(struct wstr const *const src, struct str *const dest) { return to_cp(CP_ACP, src, dest); }

error to_utf8(struct wstr const *const src, struct str *const dest) { return to_cp(CP_UTF8, src, dest); }
