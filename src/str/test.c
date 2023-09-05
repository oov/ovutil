#include <ovtest.h>
#include <ovutil/str.h>

#ifdef _WIN32
#  define NSTR_PH "ls"
#  define NATIVE_CMP wcscmp
#  define NATIVE_ONE(UTF8, WIDE) WIDE
#else
#  define NSTR_PH "s"
#  define NATIVE_CMP strcmp
#  define NATIVE_ONE(UTF8, WIDE) UTF8
#endif

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
    {"test_include_trailing_path_delimiter", test_include_trailing_path_delimiter},
    {"test_exclude_trailing_path_delimiter", test_exclude_trailing_path_delimiter},
    {"test_extract_file_name", test_extract_file_name},
    {"test_extract_file_extension", test_extract_file_extension},
    {NULL, NULL},
};
