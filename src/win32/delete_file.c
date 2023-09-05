#include <ovutil/win32.h>

NODISCARD error delete_file(struct wstr const *const path) {
  if (!DeleteFileW(path->ptr)) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  return eok();
}
