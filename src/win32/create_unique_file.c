#include "ovnum.h"
#include "ovutil/win32.h"

NODISCARD error create_unique_file(wchar_t const *const base_fullpath,
                                   wchar_t const *const ext,
                                   DWORD const file_attributes,
                                   void *const data,
                                   size_t const datalen,
                                   struct wstr *const dest) {
  if (!base_fullpath || !ext) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  struct wstr tmp = {0};
  error err = scpy(&tmp, base_fullpath);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

  size_t const base_fullpath_len = tmp.len;
  uint32_t hash = ov_splitmix32_next(GetTickCount() + GetCurrentProcessId() + GetCurrentThreadId());
  wchar_t numbuf[32] = {0};
  wchar_t *numstr = numbuf;
  for (int i = 0; i < 9; ++i) {
    err = scatm(&tmp, numstr, ext);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    HANDLE file =
        CreateFileW(tmp.ptr, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, file_attributes, NULL);
    if (file == INVALID_HANDLE_VALUE) {
      HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
      if (hr == HRESULT_FROM_WIN32(ERROR_FILE_EXISTS)) {
        tmp.ptr[base_fullpath_len] = L'\0';
        numstr = ov_utoa((uint64_t)(ov_splitmix32(hash) & 0xffffff), numbuf);
        *--numstr = L'.';
        hash = ov_splitmix32_next(hash);
        continue;
      }
      err = errhr(hr);
      goto cleanup;
    }
    if (data != NULL && datalen > 0) {
      size_t written = 0;
      err = write_file(file, data, datalen, &written);
      if (efailed(err)) {
        err = ethru(err);
        CloseHandle(file);
        DeleteFileW(tmp.ptr);
        goto cleanup;
      }
      if (written != datalen) {
        err = errg(err_fail);
        CloseHandle(file);
        DeleteFileW(tmp.ptr);
        goto cleanup;
      }
    }
    CloseHandle(file);

    err = scpy(dest, tmp.ptr);
    if (efailed(err)) {
      DeleteFileW(tmp.ptr);
      err = ethru(err);
      goto cleanup;
    }
    goto cleanup;
  }

  err = errhr(HRESULT_FROM_WIN32(ERROR_FILE_EXISTS));

cleanup:
  ereport(sfree(&tmp));
  return err;
}
