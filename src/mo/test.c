#include "mo.c"
#include "ovtest.h"

#include <stdio.h>

static struct mo *open_mo(void) {
  FILE *f = fopen(SOURCE_DIR "/mo/test/en_US.mo", "rb");
  if (!TEST_CHECK(f != NULL)) {
    return NULL;
  }
  uint8_t mobuf[4096];
  size_t mosize = fread(mobuf, 1, 4096, f);
  fclose(f);
  if (!TEST_CHECK(mosize > 0)) {
    return NULL;
  }
  struct mo *mo = NULL;
  if (!TEST_SUCCEEDED_F(mo_parse(&mo, mobuf, mosize))) {
    return NULL;
  }
  return mo;
}

static void test_mo(void) {
  struct mo *mp = open_mo();
  if (!mp) {
    return;
  }
  struct mo_msg *msg = mp->msg;
  TEST_CHECK(msg->id != NULL && msg->id_len == 0); // header
  msg = mp->msg + 1;
  TEST_CHECK(msg->id != NULL && memcmp(msg->id, "Hello world", msg->id_len) == 0);
  TEST_CHECK(msg->str != NULL && memcmp(msg->str, "Hello world2", msg->str_len) == 0);
  msg = mp->msg + 2;
  TEST_CHECK(msg->id != NULL && memcmp(msg->id, "Menu|File\x04Open", msg->id_len) == 0);
  TEST_CHECK(msg->str != NULL && memcmp(msg->str, "Open file", msg->str_len) == 0);
  msg = mp->msg + 3;
  TEST_CHECK(msg->id != NULL && memcmp(msg->id, "Menu|Printer\x04Open", msg->id_len) == 0);
  TEST_CHECK(msg->str != NULL && memcmp(msg->str, "Open printer", msg->str_len) == 0);
  msg = mp->msg + 4;
  TEST_CHECK(msg->id != NULL && memcmp(msg->id, "an apple\0apples", msg->id_len) == 0);
  TEST_CHECK(msg->str != NULL && memcmp(msg->str, "an apple2\0apples2", msg->str_len) == 0);

  msg = find(mp, "Hello world");
  TEST_CHECK(msg == mp->msg + 1);
  msg = find(mp, "Menu|Printer\x04Open");
  TEST_CHECK(msg == mp->msg + 3);
  msg = find(mp, "an apple\0apples");
  TEST_CHECK(msg == mp->msg + 4);
  mo_free(&mp);
}

static void test_public_api(void) {
  struct mo *mp = open_mo();
  if (!mp) {
    return;
  }
  char const *got = NULL, *expected = NULL;
  TEST_CHECK(strcmp(expected = "Hello world2", got = mo_gettext(mp, "Hello world")) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "Hello_world", got = mo_gettext(mp, "Hello_world")) == 0);
  TEST_MSG("expected %s got %s", expected, got);

  TEST_CHECK(strcmp(expected = "Open file", got = mo_pgettext(mp, "Menu|File", "Open")) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "Open printer", got = mo_pgettext(mp, "Menu|Printer", "Open")) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "Open", got = mo_pgettext(mp, "Menu|XXX", "Open")) == 0);
  TEST_MSG("expected %s got %s", expected, got);

  TEST_CHECK(strcmp(expected = "apples2", got = mo_ngettext(mp, "an apple", "apples", 0)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "an apple2", got = mo_ngettext(mp, "an apple", "apples", 1)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples2", got = mo_ngettext(mp, "an apple", "apples", 2)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples2", got = mo_ngettext(mp, "an apple", "apples3", 0)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "an apple2", got = mo_ngettext(mp, "an apple", "apples3", 1)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples2", got = mo_ngettext(mp, "an apple", "apples3", 2)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples", got = mo_ngettext(mp, "an apple3", "apples", 0)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "an apple3", got = mo_ngettext(mp, "an apple3", "apples", 1)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples", got = mo_ngettext(mp, "an apple3", "apples", 2)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples3", got = mo_ngettext(mp, "an apple3", "apples3", 0)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "an apple3", got = mo_ngettext(mp, "an apple3", "apples3", 1)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples3", got = mo_ngettext(mp, "an apple3", "apples3", 2)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  mo_free(&mp);
}

TEST_LIST = {
    {"test_mo", test_mo},
    {"test_public_api", test_public_api},
    {NULL, NULL},
};
