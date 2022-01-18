#include "ovtest.h"
#include "ovutil/str.h"

#ifdef __GNUC__
#  if __has_warning("-Wpadded")
#    pragma GCC diagnostic ignored "-Wpadded"
#  endif
#endif // __GNUC__

#ifdef _WIN32
#  define NSTR_PH "ls"
#  define NATIVE_CMP wcscmp
#  define NATIVE_ONE(UTF8, WIDE) WIDE
#else
#  define NSTR_PH "s"
#  define NATIVE_CMP strcmp
#  define NATIVE_ONE(UTF8, WIDE) UTF8
#endif

static void test_atoi64(void) {
  static const struct test_data {
    NATIVE_CHAR const *input;
    int64_t output;
    int code;
  } test_data[] = {
      {
          .input = NSTR("0"),
          .output = INT64_C(0),
          .code = 0,
      },
      {
          .input = NSTR("1"),
          .output = INT64_C(1),
          .code = 0,
      },
      {
          .input = NSTR("-1"),
          .output = INT64_C(-1),
          .code = 0,
      },
      {
          .input = NSTR("9223372036854775807"),
          .output = INT64_MAX,
          .code = 0,
      },
      {
          .input = NSTR("9223372036854775808"),
          .code = err_fail,
      },
      {
          .input = NSTR("-9223372036854775808"),
          .output = INT64_MIN,
          .code = 0,
      },
      {
          .input = NSTR("-9223372036854775809"),
          .code = err_fail,
      },
      {
          .input = NSTR("0x0"),
          .code = err_fail,
      },
      {
          .input = NSTR("hello"),
          .code = err_fail,
      },
  };

  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" NSTR_PH "\"", i, td->input);
    int64_t r = 0;
    if (TEST_EISG_F(atoi64(&native_unmanaged_const(td->input), &r), td->code) && td->code == 0) {
      TEST_CHECK(r == td->output);
    }
  }
}

static void test_atou64(void) {
  static const struct test_data {
    NATIVE_CHAR const *input;
    uint64_t output;
    int code;
  } test_data[] = {
      {
          .input = NSTR("0"),
          .output = UINT64_C(0),
          .code = 0,
      },
      {
          .input = NSTR("18446744073709551615"),
          .output = UINT64_MAX,
          .code = 0,
      },
      {
          .input = NSTR("18446744073709551616"),
          .code = err_fail,
      },
      {
          .input = NSTR("-1"),
          .code = err_fail,
      },
      {
          .input = NSTR("0x0"),
          .code = err_fail,
      },
      {
          .input = NSTR("hello"),
          .code = err_fail,
      },
  };

  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" NSTR_PH "\"", i, td->input);
    uint64_t r = 0;
    if (TEST_EISG_F(atou64(&native_unmanaged_const(td->input), &r), td->code) && td->code == 0) {
      TEST_CHECK(r == td->output);
    }
  }
}

static void test_itoa64(void) {
  static const struct test_data {
    int64_t input;
    NATIVE_CHAR const *output;
  } test_data[] = {
      {
          .input = INT64_C(0),
          .output = NSTR("0"),
      },
      {
          .input = INT64_C(1),
          .output = NSTR("1"),
      },
      {
          .input = INT64_MAX,
          .output = NSTR("9223372036854775807"),
      },
      {
          .input = INT64_C(-1),
          .output = NSTR("-1"),
      },
      {
          .input = INT64_MIN,
          .output = NSTR("-9223372036854775808"),
      },
  };
  struct NATIVE_STR tmp = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" NSTR_PH "\"", i, td->output);
    if (TEST_SUCCEEDED_F(itoa64(td->input, &tmp))) {
      TEST_CHECK(tmp.len > 0);
      TEST_CHECK(NATIVE_CMP(tmp.ptr, td->output) == 0);
      TEST_MSG("expected %" NSTR_PH, td->output);
      TEST_MSG("got %" NSTR_PH, tmp.ptr);
    }
  }
  ereport(sfree(&tmp));
}

static void test_utoa64(void) {
  static const struct test_data {
    uint64_t input;
    NATIVE_CHAR const *output;
  } test_data[] = {
      {
          .input = UINT64_C(0),
          .output = NSTR("0"),
      },
      {
          .input = UINT64_MAX,
          .output = NSTR("18446744073709551615"),
      },
  };

  struct NATIVE_STR tmp = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" NSTR_PH "\"", i, td->output);
    if (TEST_SUCCEEDED_F(utoa64(td->input, &tmp))) {
      TEST_CHECK(tmp.len > 0);
      TEST_CHECK(NATIVE_CMP(tmp.ptr, td->output) == 0);
      TEST_MSG("expected %" NSTR_PH, td->output);
      TEST_MSG("got %" NSTR_PH, tmp.ptr);
    }
  }
  ereport(sfree(&tmp));
}

static void test_include_trailing_path_delimiter(void) {
  static const struct test_data {
    NATIVE_CHAR const *input;
    NATIVE_CHAR const *output;
  } test_data[] = {
      {
          .input = NSTR("hello"),
          .output = NATIVE_ONE(NSTR("hello/"), NSTR("hello\\")),
      },
      {
          .input = NSTR("hello\\"),
          .output = NATIVE_ONE(NSTR("hello\\/"), NSTR("hello\\")),
      },
      {
          .input = NSTR("hello/"),
          .output = NSTR("hello/"),
      },
      {
          .input = NSTR("hello\\world"),
          .output = NATIVE_ONE(NSTR("hello\\world/"), NSTR("hello\\world\\")),
      },
      {
          .input = NSTR("hello\\world\\"),
          .output = NATIVE_ONE(NSTR("hello\\world\\/"), NSTR("hello\\world\\")),
      },
      {
          .input = NSTR("hello\\world/"),
          .output = NATIVE_ONE(NSTR("hello\\world/"), NSTR("hello\\world/")),
      },
      {
          .input = NSTR("hello/world"),
          .output = NATIVE_ONE(NSTR("hello/world/"), NSTR("hello/world\\")),
      },
      {
          .input = NSTR("hello/world\\"),
          .output = NATIVE_ONE(NSTR("hello/world\\/"), NSTR("hello/world\\")),
      },
      {
          .input = NSTR("hello/world/"),
          .output = NSTR("hello/world/"),
      },
  };

  struct NATIVE_STR ws = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" NSTR_PH "\"", i, td->input);
    TEST_SUCCEEDED_F(scpy(&ws, td->input));
    if (TEST_SUCCEEDED_F(include_trailing_path_delimiter(&ws))) {
      TEST_CHECK(ws.ptr[ws.len] == L'\0');
      TEST_CHECK(NATIVE_CMP(ws.ptr, td->output) == 0);
      TEST_MSG("expected %" NSTR_PH, td->output);
      TEST_MSG("got %" NSTR_PH, ws.ptr);
    }
  }
  TEST_SUCCEEDED_F(sfree(&ws));
  TEST_EISG_F(include_trailing_path_delimiter(NULL), err_invalid_arugment);
  TEST_EISG_F(include_trailing_path_delimiter(&ws), err_invalid_arugment);
}

static void test_exclude_trailing_path_delimiter(void) {
  static const struct test_data {
    NATIVE_CHAR const *input;
    NATIVE_CHAR const *output;
  } test_data[] = {
      {
          .input = NSTR("hello"),
          .output = NSTR("hello"),
      },
      {
          .input = NSTR("hello\\"),
          .output = NATIVE_ONE(NSTR("hello\\"), NSTR("hello")),
      },
      {
          .input = NSTR("hello/"),
          .output = NSTR("hello"),
      },
      {
          .input = NSTR("hello\\world"),
          .output = NSTR("hello\\world"),
      },
      {
          .input = NSTR("hello\\world\\"),
          .output = NATIVE_ONE(NSTR("hello\\world\\"), NSTR("hello\\world")),
      },
      {
          .input = NSTR("hello\\world/"),
          .output = NSTR("hello\\world"),
      },
      {
          .input = NSTR("hello/world"),
          .output = NSTR("hello/world"),
      },
      {
          .input = NSTR("hello/world/"),
          .output = NSTR("hello/world"),
      },
      {
          .input = NSTR("hello/world\\"),
          .output = NATIVE_ONE(NSTR("hello/world\\"), NSTR("hello/world")),
      },
  };

  struct NATIVE_STR ws = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" NSTR_PH "\"", i, td->input);
    TEST_SUCCEEDED_F(scpy(&ws, td->input));
    if (TEST_SUCCEEDED_F(exclude_trailing_path_delimiter(&ws))) {
      TEST_CHECK(ws.ptr[ws.len] == L'\0');
      TEST_CHECK(NATIVE_CMP(ws.ptr, td->output) == 0);
      TEST_MSG("expected %" NSTR_PH, td->output);
      TEST_MSG("got %" NSTR_PH, ws.ptr);
    }
  }
  TEST_SUCCEEDED_F(sfree(&ws));
  TEST_EISG_F(exclude_trailing_path_delimiter(NULL), err_invalid_arugment);
  TEST_EISG_F(exclude_trailing_path_delimiter(&ws), err_invalid_arugment);
}

static void test_extract_file_name(void) {
  static const struct test_data {
    NATIVE_CHAR const *input;
    size_t output;
  } test_data[] = {
      {
          .input = NSTR("your.exe"),
          .output = 0,
      },
      {
          .input = NSTR("C:\\test\\your.exe"),
          .output = NATIVE_ONE(0, 8),
      },
      {
          .input = NSTR("C:/test/your.exe"),
          .output = 8,
      },
      {
          .input = NSTR("C:\\test/your.exe"),
          .output = 8,
      },
      {
          .input = NSTR("C:/test\\your.exe"),
          .output = NATIVE_ONE(3, 8),
      },
  };
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" NSTR_PH "\"", i, td->input);
    struct NATIVE_STR src = native_unmanaged_const(td->input);
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
    NATIVE_CHAR const *input;
    size_t output;
  } test_data[] = {
      {
          .input = NSTR("your"),
          .output = 4,
      },
      {
          .input = NSTR("your.exe"),
          .output = 4,
      },
      {
          .input = NSTR("your.tmp.exe"),
          .output = 8,
      },
      {
          .input = NSTR("C:\\test\\your"),
          .output = 12,
      },
      {
          .input = NSTR("C:\\test\\your.exe"),
          .output = 12,
      },
      {
          .input = NSTR("C:\\test\\your.tmp.exe"),
          .output = 16,
      },
      {
          .input = NSTR("C:/test/your"),
          .output = 12,
      },
      {
          .input = NSTR("C:/test/your.exe"),
          .output = 12,
      },
      {
          .input = NSTR("C:/test/your.tmp.exe"),
          .output = 16,
      },
      {
          .input = NSTR("C:\\test/your"),
          .output = 12,
      },
      {
          .input = NSTR("C:\\test/your.exe"),
          .output = 12,
      },
      {
          .input = NSTR("C:\\test/your.tmp.exe"),
          .output = 16,
      },
      {
          .input = NSTR("C:/test\\your"),
          .output = 12,
      },
      {
          .input = NSTR("C:/test\\your.exe"),
          .output = 12,
      },
      {
          .input = NSTR("C:/test\\your.tmp.exe"),
          .output = 16,
      },

      {
          .input = NSTR("C:\\test.dir\\your"),
          .output = NATIVE_ONE(7, 16),
      },
      {
          .input = NSTR("C:\\test.dir\\your.exe"),
          .output = 16,
      },
      {
          .input = NSTR("C:\\test.dir\\your.tmp.exe"),
          .output = 20,
      },
      {
          .input = NSTR("C:/test.dir/your"),
          .output = 16,
      },
      {
          .input = NSTR("C:/test.dir/your.exe"),
          .output = 16,
      },
      {
          .input = NSTR("C:/test.dir/your.tmp.exe"),
          .output = 20,
      },
      {
          .input = NSTR("C:\\test.dir/your"),
          .output = 16,
      },
      {
          .input = NSTR("C:\\test.dir/your.exe"),
          .output = 16,
      },
      {
          .input = NSTR("C:\\test.dir/your.tmp.exe"),
          .output = 20,
      },
      {
          .input = NSTR("C:/test.dir\\your"),
          .output = NATIVE_ONE(7, 16),
      },
      {
          .input = NSTR("C:/test.dir\\your.exe"),
          .output = 16,
      },
      {
          .input = NSTR("C:/test.dir\\your.tmp.exe"),
          .output = 20,
      },
  };
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" NSTR_PH "\"", i, td->input);
    struct NATIVE_STR const src = native_unmanaged_const(td->input);
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
    {"test_itoa64", test_itoa64},
    {"test_utoa64", test_utoa64},
    {"test_include_trailing_path_delimiter", test_include_trailing_path_delimiter},
    {"test_exclude_trailing_path_delimiter", test_exclude_trailing_path_delimiter},
    {"test_extract_file_name", test_extract_file_name},
    {"test_extract_file_extension", test_extract_file_extension},
    {NULL, NULL},
};
