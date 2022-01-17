#include "ovutil/win32.h"

#include "ovutil/str.h"

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

error delete_file(struct wstr const *const path) {
  if (!DeleteFileW(path->ptr)) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  return eok();
}
