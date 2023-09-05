#include <ovutil/win32.h>

#include <shlwapi.h>

NODISCARD error path_relative_path_to(struct wstr const *const path_from,
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
