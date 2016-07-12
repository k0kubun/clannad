#include "test_helper.h"

void test_float_var() {
  float a = 1.1;
  pass();
}

void test_float_macro() {
# define A 1.1
  float a = A;
  int n = 0;
# if A
  n++;
# endif
  expect_int(n, 1);
}

int main() {
  test_float_var();
  return 0;
}
