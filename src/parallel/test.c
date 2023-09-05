#include <ovtest.h>
#include <ovutil/parallel.h>

static void test_get_number_of_physical_cores(void) {
  size_t n = 0;
  TEST_SUCCEEDED_F(get_number_of_physical_cores(&n));
  TEST_CHECK(n > 0);
  TEST_MSG("expected > 0");
  TEST_MSG("got %zu", n);
}

static void test_get_number_of_logical_cores(void) {
  size_t n = 0;
  TEST_SUCCEEDED_F(get_number_of_logical_cores(&n));
  TEST_CHECK(n > 0);
  TEST_MSG("expected > 0");
  TEST_MSG("got %zu", n);
}

enum {
  threads = 4,
  task_n = 13,
};

static void task(void *userdata, size_t const index, size_t const total) {
  size_t *found = userdata;
  TEST_CHECK(total == task_n);
  TEST_CHECK(index < task_n);
  ++found[index];
}

static void test_ovparallel(void) {
  struct ovparallel *para = NULL;
  if (!TEST_SUCCEEDED_F(ovparallel_create(&para, threads))) {
    goto cleanup;
  }
  size_t found[task_n] = {0};
  ovparallel_for(para, task, found, task_n);
  for (int i = 0; i < task_n; ++i) {
    TEST_CHECK(found[i] == 1);
    TEST_MSG("index: %d -> %d", i, (int)found[i]);
  }

cleanup:
  if (para) {
    ereport(ovparallel_destroy(&para));
  }
}

TEST_LIST = {
    {"test_get_number_of_physical_cores", test_get_number_of_physical_cores},
    {"test_get_number_of_logical_cores", test_get_number_of_logical_cores},
    {"test_ovparallel", test_ovparallel},
    {NULL, NULL},
};
