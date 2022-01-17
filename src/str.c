#include "ovutil/str.h"

error sanitize(struct wstr *const ws) {
  if (!ws) {
    return errg(err_invalid_arugment);
  }
  for (wchar_t *s = ws->ptr, *end = ws->ptr + ws->len; s < end; ++s) {
    wchar_t const c = *s;
    if (c <= 0x1f || c == 0x22 || c == 0x2a || c == 0x2b || c == 0x2f || c == 0x3a || c == 0x3c || c == 0x3e ||
        c == 0x3f || c == 0x7c || c == 0x7f) {
      *s = L'-';
    }
  }
  return eok();
}

error include_trailing_path_delimiter(struct wstr *const ws) {
  if (!ws || ws->len == 0) {
    return errg(err_invalid_arugment);
  }
  if (ws->ptr[ws->len - 1] == L'\\' || ws->ptr[ws->len - 1] == L'/') {
    return eok();
  }
  error err = scat(ws, L"\\");
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}

error exclude_trailing_path_delimiter(struct wstr *const ws) {
  if (!ws || ws->len == 0) {
    return errg(err_invalid_arugment);
  }
  if (ws->ptr[ws->len - 1] != L'\\' && ws->ptr[ws->len - 1] != L'/') {
    return eok();
  }
  ws->ptr[--ws->len] = L'\0';
  return eok();
}

#define max(a, b) (a > b ? a : b)

error extract_file_name(struct wstr const *const src, size_t *const pos) {
  if (!src) {
    return errg(err_invalid_arugment);
  }
  if (!pos) {
    return errg(err_null_pointer);
  }
  if (!src->ptr) {
    *pos = 0;
    return eok();
  }
  wchar_t const *const bslash = wcsrchr(src->ptr, L'\\');
  wchar_t const *const slash = wcsrchr(src->ptr, L'/');
  if (bslash == NULL && slash == NULL) {
    *pos = 0;
    return eok();
  }
  if (bslash != NULL && slash != NULL) {
    *pos = (size_t)(max(bslash, slash) + 1 - src->ptr);
    return eok();
  }

  *pos = (size_t)((bslash != NULL ? bslash : slash) + 1 - src->ptr);
  return eok();
}

error extract_file_extension(struct wstr const *const src, size_t *const pos) {
  if (!src) {
    return errg(err_invalid_arugment);
  }
  if (!pos) {
    return errg(err_null_pointer);
  }
  size_t fnpos = 0;
  error err = extract_file_name(src, &fnpos);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  wchar_t const *const dot = wcsrchr(src->ptr + fnpos, L'.');
  if (dot == NULL) {
    *pos = src->len;
    return eok();
  }
  *pos = (size_t)(dot - src->ptr);
  return eok();
}
