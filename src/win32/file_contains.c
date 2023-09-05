#include <ovutil/win32.h>

NODISCARD error file_contains(struct wstr const *const dir, struct wstr const *const file, bool *const contains) {
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
