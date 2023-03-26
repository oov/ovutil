#pragma once

#include "ovbase.h"

struct mo;

NODISCARD error mo_parse(struct mo **const mpp, void const *const ptr, size_t const ptrlen);
void mo_free(struct mo **const mpp);
char const *mo_gettext(struct mo const *const mp, char const *const id);
char const *mo_pgettext(struct mo const *const mp, char const *const ctxt, char const *const id);
char const *
mo_ngettext(struct mo const *const mp, char const *const id, char const *const id_plural, unsigned long int const n);

void mo_set_default(struct mo *const mp);
struct mo *mo_get_default(void);

#ifdef _WIN32
NODISCARD error mo_parse_from_resource(struct mo **const mpp, uint16_t const lang_id);
int mo_sprintf(wchar_t *const buf, size_t const buflen, wchar_t const *const reference, char const *const format, ...);
NODISCARD error mo_sprintf_wstr(struct wstr *const dest, wchar_t const *const reference, char const *const format, ...);
#endif
