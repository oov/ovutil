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

error sanitize(struct wstr *const ws) {
  if (!ws) {
    return errg(err_invalid_arugment);
  }
  for (wchar_t *s = ws->ptr, *end = ws->ptr + ws->len; s < end; ++s) {
    wchar_t const c = *s;
    if (c <= 0x1f || c == 0x22 || c == 0x2a || c == 0x2b || c == 0x2f || c == 0x3a || c == 0x3c || c == 0x3e ||
        c == 0x3f || c == 0x7c || c == 0x7f) {
      *s = L'-';
    }
  }
  return eok();
}

error include_trailing_path_delimiter(struct wstr *const ws) {
  if (!ws || ws->len == 0) {
    return errg(err_invalid_arugment);
  }
  if (ws->ptr[ws->len - 1] == L'\\' || ws->ptr[ws->len - 1] == L'/') {
    return eok();
  }
  error err = scat(ws, L"\\");
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}

error exclude_trailing_path_delimiter(struct wstr *const ws) {
  if (!ws || ws->len == 0) {
    return errg(err_invalid_arugment);
  }
  if (ws->ptr[ws->len - 1] != L'\\' && ws->ptr[ws->len - 1] != L'/') {
    return eok();
  }
  ws->ptr[--ws->len] = L'\0';
  return eok();
}

#define max(a, b) (a > b ? a : b)

error extract_file_name(struct wstr const *const src, size_t *const pos) {
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

error extract_file_extension(struct wstr const *const src, size_t *const pos) {
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
