#include "ovutil/str.h"

NODISCARD error exclude_trailing_path_delimiter(struct NATIVE_STR *const s) {
  if (!s || s->len == 0) {
    return errg(err_invalid_arugment);
  }
#ifdef _WIN32
  if (s->ptr[s->len - 1] != NSTR('\\') && s->ptr[s->len - 1] != NSTR('/')) {
    return eok();
  }
#else
  if (s->ptr[s->len - 1] != NSTR('/')) {
    return eok();
  }
#endif
  s->ptr[--s->len] = NSTR('\0');
  return eok();
}
