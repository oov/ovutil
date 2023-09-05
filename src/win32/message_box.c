#include <ovutil/win32.h>

int message_box(HWND const window, wchar_t const *const msg, wchar_t const *const title, UINT const flags) {
  HWND *w = NULL;
  error err = disable_family_windows(window, &w);
  if (efailed(err)) {
    efree(&err);
  }
  int const r = MessageBoxW(window, msg, title, flags);
  restore_disabled_family_windows(w);
  return r;
}
