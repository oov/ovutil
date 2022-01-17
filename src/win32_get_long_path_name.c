#include "ovutil/win32.h"

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
