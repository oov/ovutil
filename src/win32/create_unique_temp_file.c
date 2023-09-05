#include <ovutil/str.h>
#include <ovutil/win32.h>

NODISCARD error create_unique_temp_file(wchar_t const *const base_filename,
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
