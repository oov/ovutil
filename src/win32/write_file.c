#include <ovutil/win32.h>

NODISCARD error write_file(HANDLE const h, void *const p, size_t const bytes, size_t *const written) {
  if (!h || !p) {
    return errg(err_invalid_arugment);
  }
  size_t written_total = 0;
  char *ptr = p;
  DWORD sz = 0, ret = 0;
  error err = eok();
  while (written_total < bytes) {
    if (sizeof(DWORD) < sizeof(size_t)) {
      sz = (bytes - written_total > UINT32_MAX) ? UINT32_MAX : (DWORD)(bytes - written_total);
    } else {
      sz = (DWORD)(bytes - written_total);
    }
    if (!WriteFile(h, ptr + written_total, sz, &ret, NULL)) {
      err = errhr(HRESULT_FROM_WIN32(GetLastError()));
      goto cleanup;
    }
    written_total += (size_t)ret;
  }
cleanup:
  if (written) {
    *written = written_total;
  }
  return err;
}
