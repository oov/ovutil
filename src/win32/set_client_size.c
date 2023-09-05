#include <ovutil/win32.h>

NODISCARD error set_client_size(HWND const window, LONG const width, LONG const height) {
  RECT wr = {0};
  if (!GetWindowRect(window, &wr)) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  RECT cr = {0};
  if (!GetClientRect(window, &cr)) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  if (!SetWindowPos(window,
                    NULL,
                    0,
                    0,
                    (wr.right - wr.left) - (cr.right - cr.left) + width,
                    (wr.bottom - wr.top) - (cr.bottom - cr.top) + height,
                    SWP_NOMOVE | SWP_NOZORDER)) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  return eok();
}
