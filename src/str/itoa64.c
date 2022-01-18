#include "ovutil/str.h"

NODISCARD error itoa64(int64_t v, struct wstr *const dest) {
  if (!dest) {
    return errg(err_null_pointer);
  }
  enum {
    buf_size = 32,
  };
  wchar_t buf[buf_size] = {0};
  size_t i = buf_size - 1;
  bool const minus = v < 0;
  uint64_t w = (uint64_t)(minus ? -v : v);
  while (w > 9) {
    buf[i--] = (w % 10) + L'0';
    w /= 10;
  }
  buf[i] = (wchar_t)w + L'0';
  if (minus) {
    buf[--i] = L'-';
  }
  error err = sncpy(dest, buf + i, buf_size - i);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}
