#include "ovutil/parallel.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <stdio.h>
#endif // _WIN32

#ifdef _WIN32
NODISCARD error get_number_of_physical_cores(size_t *const n) {
  if (!n) {
    return errg(err_null_pointer);
  }
  error err = eok();
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION p = NULL;
  DWORD len = 0;
  for (;;) {
    if (GetLogicalProcessorInformation(p, &len)) {
      break;
    }
    HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
    if (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
      err = errhr(hr);
      goto cleanup;
    }
    if (p) {
      ereport(mem_free(&p));
    }
    err = mem(&p, len, 1);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
  }
  size_t cores = 0;
  DWORD end = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
  for (size_t i = 0; end <= len; ++i, end += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)) {
    if (p[i].Relationship == RelationProcessorCore) {
      ++cores;
    }
  }
  *n = cores;

cleanup:
  if (p) {
    ereport(mem_free(&p));
  }
  return err;
}

#else
static inline int isdigit(int c) { return '0' <= c && c <= '9'; }
static char *extract_digits(char *const buf, size_t const bufsize, size_t offset) {
  for (; offset < bufsize; ++offset) {
    if (isdigit(buf[offset])) {
      break;
    }
  }
  for (size_t i = offset; i < bufsize; ++i) {
    if (!isdigit(buf[i])) {
      buf[i] = '\0';
      break;
    }
  }
  return buf + offset;
}

NODISCARD error get_number_of_physical_cores(size_t *const n) {
  if (!n) {
    return errg(err_null_pointer);
  }
  struct core {
    int pid;
    int cid;
  };
  struct hmap cores = {0};
  FILE *fp = NULL;
  error err = hmnews(&cores, sizeof(struct core), 0, sizeof(struct core));
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  fp = fopen("/proc/cpuinfo", "r");
  if (!fp) {
    err = errg(err_fail);
    goto cleanup;
  }
  enum {
    bufsize = 256,
  };
  char s[bufsize];
  struct core c;
  static char const pid_key[] = "physical id\t";
  size_t const pid_key_len = strlen(pid_key);
  static char const cid_key[] = "core id\t";
  size_t const cid_key_len = strlen(cid_key);

  int64_t v;
  bool found_pid = false, found_cid = false;
  while (fgets(s, bufsize, fp)) {
    char *sep = strchr(s, ':');
    if (!sep) {
      continue;
    }
    if (strncmp(s, pid_key, pid_key_len) == 0) {
      err = satoi(&str_unmanaged_const(extract_digits(s, bufsize, (size_t)(sep - s))), &v);
      if (efailed(err)) {
        err = ethru(err);
        goto cleanup;
      }
      c.pid = (int)v;
      found_pid = true;
    }
    if (strncmp(s, cid_key, cid_key_len) == 0) {
      err = satoi(&str_unmanaged_const(extract_digits(s, bufsize, (size_t)(sep - s))), &v);
      if (efailed(err)) {
        err = ethru(err);
        goto cleanup;
      }
      c.cid = (int)v;
      found_cid = true;
    }
    if (found_pid && found_cid) {
      err = hmset(&cores, &c, NULL);
      if (efailed(err)) {
        err = ethru(err);
        goto cleanup;
      }
      found_pid = false;
      found_cid = false;
    }
  }
  err = hmcount(&cores, n);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
cleanup:
  ereport(hmfree(&cores));
  if (fp) {
    fclose(fp);
  }
  return err;
}
#endif // _WIN32
