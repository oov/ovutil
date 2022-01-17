#include "ovutil/win32.h"

NODISCARD error get_module_file_name(HINSTANCE const hinst, struct wstr *const dest) {
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
