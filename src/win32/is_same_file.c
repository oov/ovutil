#include <ovutil/win32.h>

NODISCARD static error get_file_information(struct wstr const *const path, BY_HANDLE_FILE_INFORMATION *const bhfi) {
  if (!path) {
    return errg(err_invalid_arugment);
  }
  if (!bhfi) {
    return errg(err_null_pointer);
  }
  DWORD attr = 0;
  error err = get_file_attributes(path, &attr);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  DWORD const fa = (attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY ? FILE_FLAG_BACKUP_SEMANTICS : 0;
  HANDLE const h =
      CreateFileW(path->ptr, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, fa, NULL);
  if (h == INVALID_HANDLE_VALUE) {
    return errhr(HRESULT_FROM_WIN32(GetLastError()));
  }
  BY_HANDLE_FILE_INFORMATION fi = {0};
  if (!GetFileInformationByHandle(h, &fi)) {
    HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
    CloseHandle(h);
    return errhr(hr);
  }
  CloseHandle(h);
  *bhfi = fi;
  return eok();
}

NODISCARD error is_same_file(struct wstr const *const file1, struct wstr const *const file2, bool *const same) {
  if (!file1 || !file2) {
    return errg(err_invalid_arugment);
  }
  if (!same) {
    return errg(err_null_pointer);
  }
  BY_HANDLE_FILE_INFORMATION fi1 = {0};
  BY_HANDLE_FILE_INFORMATION fi2 = {0};
  error err = get_file_information(file1, &fi1);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  err = get_file_information(file2, &fi2);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  *same = fi1.dwVolumeSerialNumber == fi2.dwVolumeSerialNumber && fi1.nFileIndexLow == fi2.nFileIndexLow &&
          fi1.nFileIndexHigh == fi2.nFileIndexHigh;
  return eok();
}

NODISCARD error is_same_dir(struct wstr const *const dir1, struct wstr const *const dir2, bool *const same) {
  return is_same_file(dir1, dir2, same);
}
