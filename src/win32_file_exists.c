#include "ovutil/win32.h"

NODISCARD error file_exists(struct wstr const *const path, bool *const exists) {
  if (!path) {
    return errg(err_invalid_arugment);
  }
  if (!exists) {
    return errg(err_null_pointer);
  }

  DWORD attr = 0;
  error err = get_file_attributes(path, &attr);
  if (efailed(err)) {
    if (eis_hr(err, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) ||
        eis_hr(err, HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))) {
      efree(&err);
      *exists = false;
      return eok();
    }
    err = ethru(err);
    return err;
  }
  *exists = true;
  return eok();
}
