#include "ovutil/win32.h"

NODISCARD error get_file_attributes(struct wstr const *const path, DWORD *const attr) {
  if (!path) {
    return errg(err_invalid_arugment);
  }
  if (!attr) {
    return errg(err_null_pointer);
  }

  DWORD const r = GetFileAttributesW(path->ptr);
  if (r == INVALID_FILE_ATTRIBUTES) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  *attr = r;
  return eok();
}
