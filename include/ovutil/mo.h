#pragma once

#include "ovbase.h"

struct mo;

NODISCARD error mo_parse(struct mo **const mpp, void const *const ptr, size_t const ptrlen);
void mo_free(struct mo **const mpp);
char const *mo_gettext(struct mo const *const mp, char const *const id);
char const *mo_pgettext(struct mo const *const mp, char const *const ctxt, char const *const id);
char const *mo_ngettext(struct mo const *const mp, char const *const id, char const *const id_plural, unsigned long int const n);
