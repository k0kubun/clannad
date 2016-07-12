#include "test_helper.h"

#define foo 3

void test_define_replacement() {
  expect_int(3, foo);
}

int main() {
  test_define_replacement();
  return 0;
}
