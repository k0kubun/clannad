#include "test_helper.h"

#define foo 3

void test_define_replacement() {
  expect_int(3, foo);
}

void test() {
  test_define_replacement();
}
