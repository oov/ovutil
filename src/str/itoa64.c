#include "ovutil/str.h"

NODISCARD error itoa64(int64_t v, struct NATIVE_STR *const dest) {
  if (!dest) {
    return errg(err_null_pointer);
  }
  enum {
    buf_size = 32,
  };
  NATIVE_CHAR buf[buf_size] = {0};
  size_t i = buf_size - 1;
  bool const minus = v < 0;
  uint64_t w = (uint64_t)(minus ? -v : v);
  while (w > 9) {
    buf[i--] = (w % 10) + NSTR('0');
    w /= 10;
  }
  buf[i] = (NATIVE_CHAR)w + NSTR('0');
  if (minus) {
    buf[--i] = NSTR('-');
  }
  error err = sncpy(dest, buf + i, buf_size - i);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}
