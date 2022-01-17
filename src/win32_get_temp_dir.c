#include "ovutil/win32.h"

NODISCARD error get_temp_dir(struct wstr *const dest) {
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
