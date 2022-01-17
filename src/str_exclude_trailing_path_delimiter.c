#include "ovutil/str.h"

NODISCARD error exclude_trailing_path_delimiter(struct wstr *const ws) {
  if (!ws || ws->len == 0) {
    return errg(err_invalid_arugment);
  }
  if (ws->ptr[ws->len - 1] != L'\\' && ws->ptr[ws->len - 1] != L'/') {
    return eok();
  }
  ws->ptr[--ws->len] = L'\0';
  return eok();
}
