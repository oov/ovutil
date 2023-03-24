#pragma once

#include "ovbase.h"

struct mo;

NODISCARD error mo_parse(struct mo **const mpp, void const *const ptr, size_t const ptrlen);
void mo_free(struct mo **const mpp);
char const *mo_gettext(struct mo const *const mp, char const *const id);
char const *mo_pgettext(struct mo const *const mp, char const *const ctxt, char const *const id);
char const *mo_ngettext(struct mo const *const mp, char const *const id, char const *const id_plural, unsigned long int const n);

void mo_set_default(struct mo  *const mp);
struct mo  * mo_get_default(void);

#ifdef _WIN32
NODISCARD error mo_parse_from_resource(struct mo **const mpp, uint16_t const lang_id);
wchar_t const *mo_gettext_win32(struct mo const *const mp, struct wstr *const buf, wchar_t const *const id);
wchar_t const *mo_pgettext_win32(struct mo const *const mp, struct wstr *const buf, wchar_t const *const ctxt, wchar_t const *const id);
wchar_t const *mo_ngettext_win32(struct mo const *const mp, struct wstr *const buf, wchar_t const *const id, wchar_t const *const id_plural, unsigned long int const n);

#endif