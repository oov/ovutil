#pragma once

#include "ovbase.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void set_hinstance(HINSTANCE const h);
HINSTANCE get_hinstance(void);

NODISCARD error disable_family_windows(HWND const exclude, HWND **const disabled_windows);
void restore_disabled_family_windows(HWND *const disabled_windows);

int message_box(HWND const window, wchar_t const *const msg, wchar_t const *const title, UINT const flags);

NODISCARD error set_client_size(HWND const window, LONG const width, LONG const height);

NODISCARD error get_window_text(HWND const window, struct wstr *const dest);
NODISCARD error get_module_file_name(HINSTANCE const hinst, struct wstr *const dest);
NODISCARD error get_temp_dir(struct wstr *const dest);
NODISCARD error get_long_path_name(struct wstr const *const src, struct wstr *const dest);
NODISCARD error file_exists(struct wstr const *const path, bool *const exists);
NODISCARD error file_contains(struct wstr const *const dir, struct wstr const *const file, bool *const contains);
NODISCARD error is_same_file(struct wstr const *const file1, struct wstr const *const file2, bool *const is_same);
NODISCARD error is_same_dir(struct wstr const *const dir1, struct wstr const *const dir2, bool *const is_same);

NODISCARD error create_unique_temp_file(wchar_t const *const base_filename,
                                        wchar_t const *const ext,
                                        void *const data,
                                        size_t const datalen,
                                        struct wstr *const dest);
NODISCARD error create_unique_file(wchar_t const *const base_fullpath,
                                   wchar_t const *const ext,
                                   DWORD const file_attributes,
                                   void *const data,
                                   size_t const datalen,
                                   struct wstr *const dest);
NODISCARD error delete_file(struct wstr const *const path);
NODISCARD error write_file(HANDLE const h, void *const p, size_t const bytes, size_t *const written);
NODISCARD error path_relative_path_to(struct wstr const *const path_from,
                                             DWORD const attr_from,
                                             struct wstr const *const path_to,
                                             DWORD const attr_to,
                                             struct wstr *const dest);
NODISCARD error get_file_attributes(struct wstr const *const path, DWORD *const attr);

NODISCARD error from_cp(UINT const code_page, struct str const *const src, struct wstr *const dest);
NODISCARD error from_mbcs(struct str const *const src, struct wstr *const dest);
NODISCARD error from_utf8(struct str const *const src, struct wstr *const dest);
NODISCARD error to_cp(UINT const code_page, struct wstr const *const src, struct str *const dest);
NODISCARD error to_mbcs(struct wstr const *const src, struct str *const dest);
NODISCARD error to_utf8(struct wstr const *const src, struct str *const dest);
