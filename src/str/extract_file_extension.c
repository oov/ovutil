#include <ovutil/str.h>

#ifdef _WIN32
#  define NATIVE_RCHR wcsrchr
#else
#  define NATIVE_RCHR strrchr
#endif

NODISCARD error extract_file_extension(struct NATIVE_STR const *const src, size_t *const pos) {
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
  NATIVE_CHAR const *const dot = NATIVE_RCHR(src->ptr + fnpos, NSTR('.'));
  if (dot == NULL) {
    *pos = src->len;
    return eok();
  }
  *pos = (size_t)(dot - src->ptr);
  return eok();
}
