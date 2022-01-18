#include "ovutil/str.h"

NODISCARD error extract_file_name(struct NATIVE_STR const *const src, size_t *const pos) {
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
#ifdef _WIN32
#  define max(a, b) (a > b ? a : b)
  wchar_t const *const bslash = wcsrchr(src->ptr, NSTR('\\'));
  wchar_t const *const slash = wcsrchr(src->ptr, NSTR('/'));
  if (bslash == NULL && slash == NULL) {
    *pos = 0;
    return eok();
  }
  if (bslash != NULL && slash != NULL) {
    *pos = (size_t)(max(bslash, slash) + 1 - src->ptr);
    return eok();
  }
  *pos = (size_t)((bslash != NULL ? bslash : slash) + 1 - src->ptr);
#else
  char const *const slash = strrchr(src->ptr, NSTR('/'));
  if (slash == NULL) {
    *pos = 0;
    return eok();
  }
  *pos = (size_t)(slash + 1 - src->ptr);
#endif
  return eok();
}
