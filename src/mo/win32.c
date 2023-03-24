#include "ovutil/win32.h"
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

wchar_t const *mo_gettext_win32(struct mo const *const mp, struct wstr *const buf, wchar_t const *const id) {
  wchar_t const *r = id;
  struct str u8 = {0};
  error err = to_utf8(&wstr_unmanaged_const(id), &u8);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  char const *const s = mo_gettext(mp, u8.ptr);
  if (s == u8.ptr) {
    goto cleanup;
  }
  err = from_utf8(&str_unmanaged_const(s), buf);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  r = buf->ptr;
cleanup:
  ereport(sfree(&u8));
  if (efailed(err)) {
    ereport(err);
  }
  return r;
}

wchar_t const *mo_pgettext_win32(struct mo const *const mp,
                                 struct wstr *const buf,
                                 wchar_t const *const ctxt,
                                 wchar_t const *const id) {
  int const ctxtlen = (int)(wcslen(ctxt));
  int const u8ctxtlen = WideCharToMultiByte(CP_UTF8, 0, ctxt, ctxtlen, NULL, 0, NULL, NULL);
  if (!u8ctxtlen) {
    ereport(errhr(HRESULT_FROM_WIN32(GetLastError())));
    return id;
  }
  int const idlen = (int)(wcslen(id));
  int const u8idlen = WideCharToMultiByte(CP_UTF8, 0, id, idlen, NULL, 0, NULL, NULL);
  if (!u8idlen) {
    ereport(errhr(HRESULT_FROM_WIN32(GetLastError())));
    return id;
  }
  wchar_t const *r = id;
  struct str u8 = {0};
  error err = sgrow(&u8, (size_t)(u8ctxtlen + 1 + u8idlen + 1));
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  if (!WideCharToMultiByte(CP_UTF8, 0, ctxt, ctxtlen, u8.ptr, u8ctxtlen, NULL, NULL)) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  u8.ptr[u8ctxtlen] = '\x04';
  if (!WideCharToMultiByte(CP_UTF8, 0, id, idlen, u8.ptr + u8ctxtlen + 1, u8idlen, NULL, NULL)) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  u8.ptr[u8ctxtlen + 1 + u8idlen] = '\0';
  u8.len = (size_t)(u8ctxtlen + 1 + u8idlen);
  char const *const s = mo_gettext(mp, u8.ptr);
  if (s == u8.ptr) {
    goto cleanup;
  }
  err = from_utf8(&str_unmanaged_const(s), buf);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  r = buf->ptr;
cleanup:
  ereport(sfree(&u8));
  if (efailed(err)) {
    ereport(err);
  }
  return r;
}

wchar_t const *mo_ngettext_win32(struct mo const *const mp,
                                 struct wstr *const buf,
                                 wchar_t const *const id,
                                 wchar_t const *const id_plural,
                                 unsigned long int const n) {
  wchar_t const *r = id;
  struct str u8 = {0};
  error err = to_utf8(&wstr_unmanaged_const(id), &u8);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  char const *const s = mo_ngettext(mp, u8.ptr, u8.ptr + 1, n);
  if (s == u8.ptr) {
    goto cleanup;
  }
  if (s == u8.ptr + 1) {
    r = id_plural;
    goto cleanup;
  }
  err = from_utf8(&str_unmanaged_const(s), buf);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  r = buf->ptr;
cleanup:
  ereport(sfree(&u8));
  if (efailed(err)) {
    ereport(err);
  }
  return r;
}
