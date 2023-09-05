#include <ovutil/win32.h>

static HINSTANCE g_hinstance = NULL;

void set_hinstance(HINSTANCE const h) { g_hinstance = h; }

HINSTANCE get_hinstance(void) { return g_hinstance; }
