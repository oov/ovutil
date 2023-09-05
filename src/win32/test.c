#include <ovtest.h>
#include <ovutil/win32.h>

static void test_file_contains(void) {
  static const struct test_data {
    wchar_t const *dir;
    wchar_t const *file;
    bool contains;
  } test_data[] = {
      {
          .dir = L"C:\\example\\dir",
          .file = L"C:\\example\\dir\\file.txt",
          .contains = true,
      },
      {
          .dir = L"C:\\example\\dir",
          .file = L"C:\\example\\file.txt",
          .contains = false,
      },
      {
          .dir = L"C:\\example\\dir\\",
          .file = L"C:\\example\\dir\\file.txt",
          .contains = true,
      },
      {
          .dir = L"C:\\example\\dir\\",
          .file = L"C:\\example\\file.txt",
          .contains = false,
      },
      {
          .dir = L"C:\\example\\DIR",
          .file = L"C:\\example\\dir\\file.txt",
          .contains = true,
      },
      {
          .dir = L"C:\\example\\DIR",
          .file = L"C:\\example\\file.txt",
          .contains = false,
      },
      {
          .dir = L"C:\\example\\DIR\\",
          .file = L"C:\\example\\dir\\file.txt",
          .contains = true,
      },
      {
          .dir = L"C:\\example\\DIR\\",
          .file = L"C:\\example\\file.txt",
          .contains = false,
      },
  };

  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%ls\"", i, td->file);
    bool contains = false;
    if (TEST_SUCCEEDED_F(file_contains(&wstr_unmanaged_const(td->dir), &wstr_unmanaged_const(td->file), &contains))) {
      TEST_CHECK(contains == td->contains);
    }
  }
}

static void test_create_unique_temp_file(void) {
  struct wstr path = {0};
  if (!TEST_SUCCEEDED_F(create_unique_temp_file(L"test_create_unique_temp_file1", L".txt", NULL, 0, &path))) {
    return;
  }
  TEST_CHECK(path.ptr != NULL);
  TEST_CHECK(path.len > 0);
  TEST_SUCCEEDED_F(delete_file(&path));
  TEST_SUCCEEDED_F(sfree(&path));

  if (!TEST_SUCCEEDED_F(create_unique_temp_file(L"test_create_unique_temp_file2", L".txt", "hello", 5, &path))) {
    return;
  }
  TEST_CHECK(path.ptr != NULL);
  HANDLE h = CreateFileW(path.ptr, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (TEST_CHECK(h != INVALID_HANDLE_VALUE)) {
    char buf[6] = {0};
    DWORD read = 0;
    if (TEST_CHECK(ReadFile(h, buf, 6, &read, NULL) == TRUE && read == 5)) {
      TEST_CHECK(strcmp(buf, "hello") == 0);
    }
    TEST_CHECK(CloseHandle(h) == TRUE);
  }
  TEST_SUCCEEDED_F(delete_file(&path));
  TEST_SUCCEEDED_F(sfree(&path));
}

static void test_from_utf8(void) {
  static struct str const src = str_unmanaged("\xE3\x81\x93\xE3\x82\x93\xE3\x81\xAB\xE3\x81\xA1\xE3\x81\xAF");
  static wchar_t const ws[] = L"こんにちは";
  size_t const len = wcslen(ws);
  struct wstr dest = {0};
  if (TEST_SUCCEEDED_F(from_utf8(&src, &dest))) {
    TEST_CHECK(wcscmp(dest.ptr, ws) == 0);
    TEST_CHECK(dest.len == len);
    TEST_MSG("expected %zu", len);
    TEST_MSG("got %zu", dest.len);
    TEST_SUCCEEDED_F(sfree(&dest));
  }
  TEST_EISG_F(from_utf8(NULL, &dest), err_invalid_arugment);
  TEST_EISG_F(from_utf8(&src, NULL), err_null_pointer);
}

static void test_to_utf8(void) {
  static struct wstr const src = wstr_unmanaged(L"こんにちは");
  static char const utf8[] = "\xE3\x81\x93\xE3\x82\x93\xE3\x81\xAB\xE3\x81\xA1\xE3\x81\xAF";
  size_t const len = strlen(utf8);
  struct str dest = {0};
  if (TEST_SUCCEEDED_F(to_utf8(&src, &dest))) {
    TEST_CHECK(strcmp(dest.ptr, utf8) == 0);
    TEST_CHECK(dest.len == len);
    ereport(sfree(&dest));
  }
  TEST_EISG_F(to_utf8(NULL, &dest), err_invalid_arugment);
  TEST_EISG_F(to_utf8(&src, NULL), err_null_pointer);
}

TEST_LIST = {
    {"test_file_contains", test_file_contains},
    {"test_create_unique_temp_file", test_create_unique_temp_file},
    {"test_from_utf8", test_from_utf8},
    {"test_to_utf8", test_to_utf8},
    {NULL, NULL},
};
