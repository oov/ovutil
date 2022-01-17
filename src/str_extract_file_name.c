#include "ovutil/str.h"

#define max(a, b) (a > b ? a : b)

NODISCARD error extract_file_name(struct wstr const *const src, size_t *const pos) {
  if (!src) {
    return errg(err_invalid_arugment);
  }
  if (!pos) {
    return errg(err_null_pointer);
  }
  if (!src->ptr) {
    *pos = 0;
    return eok();
  }
  wchar_t const *const bslash = wcsrchr(src->ptr, L'\\');
  wchar_t const *const slash = wcsrchr(src->ptr, L'/');
  if (bslash == NULL && slash == NULL) {
    *pos = 0;
    return eok();
  }
  if (bslash != NULL && slash != NULL) {
    *pos = (size_t)(max(bslash, slash) + 1 - src->ptr);
    return eok();
  }

  *pos = (size_t)((bslash != NULL ? bslash : slash) + 1 - src->ptr);
  return eok();
}
