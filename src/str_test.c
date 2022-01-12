#include "str.c"

#include "ovtest.h"

#ifdef __GNUC__
#  if __has_warning("-Wpadded")
#    pragma GCC diagnostic ignored "-Wpadded"
#  endif
#endif // __GNUC__

static void test_atoi64(void) {
  static const struct test_data {
    wchar_t const *input;
    int64_t output;
    uint_least32_t code;
  } test_data[] = {
      {
          .input = L"0",
          .output = 0,
          .code = 0,
      },
      {
          .input = L"1",
          .output = 1,
          .code = 0,
      },
      {
          .input = L"-1",
          .output = -1,
          .code = 0,
      },
      {
          .input = L"9223372036854775807",
          .output = INT64_MAX,
          .code = 0,
      },
      {
          .input = L"9223372036854775808",
          .code = err_fail,
      },
      {
          .input = L"-9223372036854775808",
          .output = INT64_MIN,
          .code = 0,
      },
      {
          .input = L"-9223372036854775809",
          .code = err_fail,
      },
      {
          .input = L"0x0",
          .code = err_fail,
      },
      {
          .input = L"hello",
          .code = err_fail,
      },
  };

  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%ls\"", i, td->input);
    int64_t r = 0;
    if (TEST_EISG_F(atoi64(&wstr_unmanaged_const(td->input), &r), td->code) && td->code == 0) {
      TEST_CHECK(r == td->output);
    }
  }
}

static void test_atou64(void) {
  static const struct test_data {
    wchar_t const *input;
    uint64_t output;
    uint_least32_t code;
  } test_data[] = {
      {
          .input = L"0",
          .output = 0,
          .code = 0,
      },
      {
          .input = L"18446744073709551615",
          .output = UINT64_MAX,
          .code = 0,
      },
      {
          .input = L"18446744073709551616",
          .code = err_fail,
      },
      {
          .input = L"-1",
          .code = err_fail,
      },
      {
          .input = L"0x0",
          .code = err_fail,
      },
      {
          .input = L"hello",
          .code = err_fail,
      },
  };

  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%ls\"", i, td->input);
    uint64_t r = 0;
    if (TEST_EISG_F(atou64(&wstr_unmanaged_const(td->input), &r), td->code) && td->code == 0) {
      TEST_CHECK(r == td->output);
    }
  }
}

static void test_utoa64(void) {
  static const struct test_data {
    uint64_t input;
    wchar_t const *output;
  } test_data[] = {
      {
          .input = 0ULL,
          .output = L"0",
      },
      {
          .input = UINT64_MAX,
          .output = L"18446744073709551615",
      },
  };

  struct wstr tmp = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%ls\"", i, td->output);
    if (TEST_SUCCEEDED_F(utoa64(td->input, &tmp))) {
      TEST_CHECK(tmp.len > 0);
      TEST_CHECK(wcscmp(tmp.ptr, td->output) == 0);
      TEST_MSG("expected %ls", td->output);
      TEST_MSG("got %ls", tmp.ptr);
    }
  }
  ereport(sfree(&tmp));
}

static void test_include_trailing_path_delimiter(void) {
  static const struct test_data {
    wchar_t const *input;
    wchar_t const *output;
  } test_data[] = {
      {
          .input = L"hello",
          .output = L"hello\\",
      },
      {
          .input = L"hello\\",
          .output = L"hello\\",
      },
      {
          .input = L"hello\\world",
          .output = L"hello\\world\\",
      },
      {
          .input = L"hello\\world\\",
          .output = L"hello\\world\\",
      },
      {
          .input = L"hello/",
          .output = L"hello/",
      },
      {
          .input = L"hello/world",
          .output = L"hello/world\\",
      },
      {
          .input = L"hello/world/",
          .output = L"hello/world/",
      },
  };

  struct wstr ws = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%ls\"", i, td->input);
    TEST_SUCCEEDED_F(scpy(&ws, td->input));
    if (TEST_SUCCEEDED_F(include_trailing_path_delimiter(&ws))) {
      TEST_CHECK(ws.ptr[ws.len] == L'\0');
      TEST_CHECK(wcscmp(ws.ptr, td->output) == 0);
      TEST_MSG("expected %ls", td->output);
      TEST_MSG("got %ls", ws.ptr);
    }
  }
  TEST_SUCCEEDED_F(sfree(&ws));
  TEST_EISG_F(include_trailing_path_delimiter(NULL), err_invalid_arugment);
  TEST_EISG_F(include_trailing_path_delimiter(&ws), err_invalid_arugment);
}

static void test_exclude_trailing_path_delimiter(void) {
  static const struct test_data {
    wchar_t const *input;
    wchar_t const *output;
  } test_data[] = {
      {
          .input = L"hello",
          .output = L"hello",
      },
      {
          .input = L"hello\\",
          .output = L"hello",
      },
      {
          .input = L"hello\\world",
          .output = L"hello\\world",
      },
      {
          .input = L"hello\\world\\",
          .output = L"hello\\world",
      },
      {
          .input = L"hello/",
          .output = L"hello",
      },
      {
          .input = L"hello/world",
          .output = L"hello/world",
      },
      {
          .input = L"hello/world/",
          .output = L"hello/world",
      },
  };

  struct wstr ws = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%ls\"", i, td->input);
    TEST_SUCCEEDED_F(scpy(&ws, td->input));
    if (TEST_SUCCEEDED_F(exclude_trailing_path_delimiter(&ws))) {
      TEST_CHECK(ws.ptr[ws.len] == L'\0');
      TEST_CHECK(wcscmp(ws.ptr, td->output) == 0);
      TEST_MSG("expected %ls", td->output);
      TEST_MSG("got %ls", ws.ptr);
    }
  }
  TEST_SUCCEEDED_F(sfree(&ws));
  TEST_EISG_F(exclude_trailing_path_delimiter(NULL), err_invalid_arugment);
  TEST_EISG_F(exclude_trailing_path_delimiter(&ws), err_invalid_arugment);
}

static void test_extract_file_name(void) {
  static const struct test_data {
    wchar_t const *input;
    size_t output;
  } test_data[] = {
      {
          .input = L"your.exe",
          .output = 0,
      },
      {
          .input = L"C:\\test\\your.exe",
          .output = 8,
      },
      {
          .input = L"C:/test/your.exe",
          .output = 8,
      },
      {
          .input = L"C:\\test/your.exe",
          .output = 8,
      },
      {
          .input = L"C:/test\\your.exe",
          .output = 8,
      },
  };
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%ls\"", i, td->input);
    struct wstr src = wstr_unmanaged_const(td->input);
    size_t pos = 0;
    if (TEST_SUCCEEDED_F(extract_file_name(&src, &pos))) {
      TEST_CHECK(pos == td->output);
      TEST_MSG("expected %zu", td->output);
      TEST_MSG("got %zu", pos);
    }
  }
}

static void test_extract_file_extension(void) {
  static const struct test_data {
    wchar_t const *input;
    size_t output;
  } test_data[] = {
      {
          .input = L"your.exe",
          .output = 4,
      },
      {
          .input = L"C:\\test\\your.exe",
          .output = 12,
      },
      {
          .input = L"C:/test/your.exe",
          .output = 12,
      },
      {
          .input = L"C:\\test/your.exe",
          .output = 12,
      },
      {
          .input = L"C:/test\\your.exe",
          .output = 12,
      },
      {
          .input = L"your.tmp.exe",
          .output = 8,
      },
      {
          .input = L"C:\\test\\your.tmp.exe",
          .output = 16,
      },
      {
          .input = L"C:/test/your.tmp.exe",
          .output = 16,
      },
      {
          .input = L"C:\\test/your.tmp.exe",
          .output = 16,
      },
      {
          .input = L"C:/test\\your.tmp.exe",
          .output = 16,
      },
      {
          .input = L"your",
          .output = 4,
      },
      {
          .input = L"C:\\test\\your",
          .output = 12,
      },
      {
          .input = L"C:/test/your",
          .output = 12,
      },
      {
          .input = L"C:\\test/your",
          .output = 12,
      },
      {
          .input = L"C:/test\\your",
          .output = 12,
      },
  };
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%ls\"", i, td->input);
    struct wstr const src = wstr_unmanaged_const(td->input);
    size_t pos = 0;
    if (TEST_SUCCEEDED_F(extract_file_extension(&src, &pos))) {
      TEST_CHECK(pos == td->output);
      TEST_MSG("expected %zu", td->output);
      TEST_MSG("got %zu", pos);
    }
  }
}

TEST_LIST = {
    {"test_atoi64", test_atoi64},
    {"test_atou64", test_atou64},
    {"test_utoa64", test_utoa64},
    {"test_include_trailing_path_delimiter", test_include_trailing_path_delimiter},
    {"test_exclude_trailing_path_delimiter", test_exclude_trailing_path_delimiter},
    {"test_extract_file_name", test_extract_file_name},
    {"test_extract_file_extension", test_extract_file_extension},
    {NULL, NULL},
};
