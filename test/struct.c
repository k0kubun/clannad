#include "test_helper.h"

void test_struct_decl() {
  struct {
    int a;
    int b;
  } a;
  a.a = 3;
  a.b = 4;
  expect_int(12, a.a * a.b);
}

int main() {
  test_struct_decl();
  return 0;
}
