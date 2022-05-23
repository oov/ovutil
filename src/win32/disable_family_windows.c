#include "ovutil/win32.h"

struct disable_family_windows_data {
  HWND *ptr;
  size_t len;
  size_t cap;

  DWORD pid;
  HWND exclude;
};

static WINBOOL CALLBACK disable_family_windows_callback(HWND const window, LPARAM const lparam) {
  struct disable_family_windows_data *const d = (void *)lparam;
  if (!IsWindowVisible(window) || !IsWindowEnabled(window) || d->exclude == window) {
    return TRUE;
  }
  DWORD pid = 0;
  GetWindowThreadProcessId(window, &pid);
  if (pid != d->pid) {
    return TRUE;
  }
  error err = apush(d, window);
  if (efailed(err)) {
    efree(&err);
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
  }

  return TRUE;
}

NODISCARD error disable_family_windows(HWND const exclude, HWND **const disabled_windows) {
  if (!disabled_windows) {
    return errg(err_null_pointer);
  }
  error err = eok();
  struct disable_family_windows_data d = {
      .pid = GetCurrentProcessId(),
      .exclude = exclude,
  };
  if (!EnumWindows(disable_family_windows_callback, (LPARAM)&d)) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto failed;
  }
  err = apush(&d, INVALID_HANDLE_VALUE);
  if (efailed(err)) {
    efree(&err);
    err = errhr(HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY));
    goto failed;
  }

  HWND const *h = d.ptr;
  while (*h != INVALID_HANDLE_VALUE) {
    EnableWindow(*h++, FALSE);
  }
  *disabled_windows = d.ptr;
  return eok();

failed:
  ereport(afree(&d));
  return err;
}

void restore_disabled_family_windows(HWND *const disabled_windows) {
  if (!disabled_windows) {
    return;
  }
  struct disable_family_windows_data d = {
      .ptr = disabled_windows,
      .len = 1,
      .cap = 1,
  };
  HWND *h = disabled_windows;
  while (*h != INVALID_HANDLE_VALUE) {
    EnableWindow(*h++, TRUE);
    d.cap = ++d.len;
  }
  ereport(afree(&d));
}
