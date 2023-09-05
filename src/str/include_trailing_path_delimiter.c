#include <ovutil/str.h>

NODISCARD error include_trailing_path_delimiter(struct NATIVE_STR *const s) {
  if (!s || s->len == 0) {
    return errg(err_invalid_arugment);
  }
#ifdef _WIN32
  if (s->ptr[s->len - 1] == NSTR('\\') || s->ptr[s->len - 1] == NSTR('/')) {
    return eok();
  }
  error err = scat(s, NSTR("\\"));
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
#else
  if (s->ptr[s->len - 1] == NSTR('/')) {
    return eok();
  }
  error err = scat(s, NSTR("/"));
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
#endif
  return eok();
}
