#include "ovutil/str.h"

NODISCARD error sanitize(struct wstr *const ws) {
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
