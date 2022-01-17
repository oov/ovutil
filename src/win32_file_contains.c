#include "ovutil/win32.h"

#include <shlwapi.h>

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
