#include "test_helper.h"

void test_sizeof() {
  expect_int(4, sizeof(int));
  expect_int(1, sizeof(char));
}

void test_static() {
  static int n = 0;
  expect_int(0, n);
}

// check extern
extern void extern_func();

int main() {
  test_sizeof();
  test_static();
  return 0;
}
