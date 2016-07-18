#include "test_helper.h"

void test_predefined_macros() {
  expect_int(4, __GNUC__);
  expect_int(1, __STDC__);
# ifndef __x86_64__
  expect_int(1, 0);
# endif
}

int main() {
  test_predefined_macros();
  return 0;
}
