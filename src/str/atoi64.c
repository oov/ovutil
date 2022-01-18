#include "ovutil/str.h"

NODISCARD error atoi64(struct NATIVE_STR const *const s, int64_t *const dest) {
  if (!s) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  uint64_t r = 0, pr = 0;
  NATIVE_CHAR *ptr = s->ptr;
  size_t len = s->len;
  int64_t sign = 1;
  if (len && (*ptr == NSTR('+') || *ptr == NSTR('-'))) {
    sign = *ptr == NSTR('-') ? -1 : 1;
    ++ptr;
    --len;
  }
  for (size_t i = 0; i < len; ++i) {
    if (i >= 19 || NSTR('0') > ptr[i] || ptr[i] > NSTR('9')) {
      return errg(err_fail);
    }
    pr = r;
    r = r * 10 + (uint64_t)(ptr[i] - NSTR('0'));
    if (r < pr) {
      return errg(err_fail);
    }
    continue;
  }
  if ((r >> 63) && ((sign > 0) || (r << 1))) {
    return errg(err_fail);
  }
  *dest = (int64_t)(r)*sign;
  return eok();
}
