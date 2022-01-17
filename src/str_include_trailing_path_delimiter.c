#include "ovutil/str.h"

NODISCARD error include_trailing_path_delimiter(struct wstr *const ws) {
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
