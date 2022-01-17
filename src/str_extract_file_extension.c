#include "ovutil/str.h"

NODISCARD error extract_file_extension(struct wstr const *const src, size_t *const pos) {
  if (!src) {
    return errg(err_invalid_arugment);
  }
  if (!pos) {
    return errg(err_null_pointer);
  }
  size_t fnpos = 0;
  error err = extract_file_name(src, &fnpos);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  wchar_t const *const dot = wcsrchr(src->ptr + fnpos, L'.');
  if (dot == NULL) {
    *pos = src->len;
    return eok();
  }
  *pos = (size_t)(dot - src->ptr);
  return eok();
}
