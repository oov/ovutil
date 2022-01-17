#include "ovutil/str.h"
#include "ovutil/win32.h"

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
