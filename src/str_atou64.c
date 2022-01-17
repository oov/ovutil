#include "ovutil/str.h"

NODISCARD error atou64(struct wstr const *const s, uint64_t *const dest) {
  if (!s) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  uint64_t r = 0, pr = 0;
  wchar_t *ptr = s->ptr;
  for (size_t i = 0, len = s->len; i < len; ++i) {
    if (i >= 20 || '0' > ptr[i] || ptr[i] > '9') {
      return errg(err_fail);
    }
    pr = r;
    r = r * 10 + (uint64_t)(ptr[i] - '0');
    if (r < pr) {
      return errg(err_fail);
    }
    continue;
  }
  *dest = r;
  return eok();
}
