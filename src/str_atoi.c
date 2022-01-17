#include "ovutil/str.h"

error atoi64(struct wstr const *const s, int64_t *const dest) {
  if (!s) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  uint64_t r = 0, pr = 0;
  wchar_t *ptr = s->ptr;
  size_t len = s->len;
  int64_t sign = 1;
  if (len && (*ptr == L'+' || *ptr == L'-')) {
    sign = *ptr == L'-' ? -1 : 1;
    ++ptr;
    --len;
  }
  for (size_t i = 0; i < len; ++i) {
    if (i >= 19 || '0' > ptr[i] || ptr[i] > '9') {
      return errg(err_fail);
    }
    pr = r;
    r = r * 10 + (uint64_t)(ptr[i] - '0');
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

error atou64(struct wstr const *const s, uint64_t *const dest) {
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

error utoa64(uint64_t v, struct wstr *const dest) {
  if (!dest) {
    return errg(err_null_pointer);
  }

  enum {
    buf_size = 32,
  };
  wchar_t buf[buf_size] = {0};
  size_t i = buf_size - 1;
  while (v > 9) {
    buf[i--] = (v % 10) + L'0';
    v /= 10;
  }
  buf[i] = (wchar_t)v + L'0';
  error err = sncpy(dest, buf + i, buf_size - i);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}
