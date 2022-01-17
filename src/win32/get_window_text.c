#include "ovutil/win32.h"

NODISCARD error get_window_text(HWND const window, struct wstr *const dest) {
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
