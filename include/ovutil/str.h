#pragma once

#include "ovbase.h"

NODISCARD error atoi64(struct wstr const *const s, int64_t *const dest);
NODISCARD error atou64(struct wstr const *const s, uint64_t *const dest);
NODISCARD error itoa64(int64_t const v, struct wstr *const dest);
NODISCARD error utoa64(uint64_t const v, struct wstr *const dest);

NODISCARD error include_trailing_path_delimiter(struct wstr *const ws);
NODISCARD error exclude_trailing_path_delimiter(struct wstr *const ws);
NODISCARD error extract_file_name(struct wstr const *const src, size_t *const pos);
NODISCARD error extract_file_extension(struct wstr const *const src, size_t *const pos);

NODISCARD error sanitize(struct wstr *const ws);
