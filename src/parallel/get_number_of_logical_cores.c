
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  ifdef __GNUC__
#    pragma GCC diagnostic push
#    if __has_warning("-Wreserved-id-macro")
#      pragma GCC diagnostic ignored "-Wreserved-id-macro"
#    endif
#    define _GNU_SOURCE
#    pragma GCC diagnostic pop
#  else
#    define _GNU_SOURCE
#  endif // __GNUC__
#  include <sched.h>
#endif // _WIN32

#include "ovutil/parallel.h"

#ifdef _WIN32

NODISCARD error get_number_of_logical_cores(size_t *const n) {
  if (!n) {
    return errg(err_null_pointer);
  }
  SYSTEM_INFO si;
  GetNativeSystemInfo(&si);
  *n = (size_t)si.dwNumberOfProcessors;
  return eok();
}

#else // _WIN32

NODISCARD error get_number_of_logical_cores(size_t *const n) {
  if (!n) {
    return errg(err_null_pointer);
  }
  cpu_set_t cs = {0};
  CPU_ZERO(&cs);
  if (sched_getaffinity(0, sizeof(cs), &cs) == -1) {
    return errerrno(errno);
  }
  *n = (size_t)(CPU_COUNT(&cs));
  return eok();
}

#endif // _WIN32
