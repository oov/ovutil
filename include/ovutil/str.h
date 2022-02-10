#pragma once

#include "ovbase.h"

NODISCARD error include_trailing_path_delimiter(struct NATIVE_STR *const ws);
NODISCARD error exclude_trailing_path_delimiter(struct NATIVE_STR *const ws);
NODISCARD error extract_file_name(struct NATIVE_STR const *const src, size_t *const pos);
NODISCARD error extract_file_extension(struct NATIVE_STR const *const src, size_t *const pos);

NODISCARD error sanitize(struct NATIVE_STR *const ws);
