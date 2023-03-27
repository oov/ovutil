#include "ovutil/win32.h"
#include "ovprintf.h"
#include "ovutil/mo.h"

NODISCARD error mo_parse_from_resource(struct mo **const mpp, uint16_t const lang_id) {
  HINSTANCE const hinst = get_hinstance();
  HRSRC r = FindResourceExW(hinst, MAKEINTRESOURCEW(10), L"MO", lang_id);
  if (r == NULL) {
    return 0;
  }
  HGLOBAL h = LoadResource(hinst, r);
  if (!h) {
    return 0;
  }
  void const *const p = LockResource(h);
  if (!p) {
    return 0;
  }
  size_t const sz = (size_t)(SizeofResource(hinst, r));
  if (!sz) {
    return 0;
  }
  error err = mo_parse(mpp, p, sz);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
cleanup:
  return err;
}

int mo_sprintf(wchar_t *const buf, size_t const buflen, wchar_t const *const reference, char const *const format, ...) {
  if (!buf || !buflen || !format) {
    return 0;
  }
  struct wstr ws = {0};
  error err = from_utf8(&str_unmanaged_const(format), &ws);
  if (efailed(err)) {
    err = ethru(err);
    ereport(err);
    buf[0] = L'\0';
    return 0;
  }
  va_list valist;
  va_start(valist, format);
  int const r = ov_vsnprintf(buf, buflen, reference, ws.ptr, valist);
  va_end(valist);
  ereport(sfree(&ws));
  return r;
}

NODISCARD error mo_vsprintf_wstr(struct wstr *const dest,
                                 wchar_t const *const reference,
                                 char const *const format,
                                 va_list valist) {
  if (!dest || !format) {
    return errg(err_invalid_arugment);
  }
  struct wstr ws = {0};
  error err = from_utf8(&str_unmanaged_const(format), &ws);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = svsprintf(dest, reference, ws.ptr, valist);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
cleanup:
  ereport(sfree(&ws));
  return err;
}

NODISCARD error mo_sprintf_wstr(struct wstr *const dest,
                                wchar_t const *const reference,
                                char const *const format,
                                ...) {
  va_list valist;
  va_start(valist, format);
  error err = mo_vsprintf_wstr(dest, reference, format, valist);
  va_end(valist);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
cleanup:
  return err;
}
