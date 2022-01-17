#include "ovutil/win32.h"

error from_cp(UINT const code_page, struct str const *const src, struct wstr *const dest) {
  if (!src) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  struct wstr tmp = {0};
  error err = eok();

  if (src->len == 0) {
    err = scpy(dest, L"");
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    return eok();
  }

  int const dlen = MultiByteToWideChar(code_page, 0, src->ptr, (int)src->len, NULL, 0);
  if (!dlen) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  err = sgrow(&tmp, (size_t)(dlen + 1));
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

  if (!MultiByteToWideChar(code_page, 0, src->ptr, (int)src->len, tmp.ptr, dlen)) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  tmp.ptr[dlen] = L'\0';
  tmp.len = (size_t)dlen;

  err = scpy(dest, tmp.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  ereport(sfree(&tmp));
  return err;
}

error to_cp(UINT const code_page, struct wstr const *const src, struct str *const dest) {
  if (!src) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  struct str tmp = {0};
  error err = eok();

  if (src->len == 0) {
    err = scpy(dest, "");
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    return eok();
  }

  int const dlen = WideCharToMultiByte(code_page, 0, src->ptr, (int)src->len, NULL, 0, NULL, NULL);
  if (!dlen) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  err = sgrow(&tmp, (size_t)(dlen + 1));
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

  if (!WideCharToMultiByte(code_page, 0, src->ptr, (int)src->len, tmp.ptr, dlen, NULL, NULL)) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  tmp.ptr[dlen] = '\0';
  tmp.len = (size_t)dlen;

  err = scpy(dest, tmp.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  ereport(sfree(&tmp));
  return err;
}

error from_mbcs(struct str const *const src, struct wstr *const dest) { return from_cp(CP_ACP, src, dest); }

error from_utf8(struct str const *const src, struct wstr *const dest) { return from_cp(CP_UTF8, src, dest); }

error to_mbcs(struct wstr const *const src, struct str *const dest) { return to_cp(CP_ACP, src, dest); }

error to_utf8(struct wstr const *const src, struct str *const dest) { return to_cp(CP_UTF8, src, dest); }
