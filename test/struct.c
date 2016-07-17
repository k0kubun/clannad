#include "test_helper.h"

void test_struct_decl() {
  struct {
    int a;
    int b;
  } a;
}

int main() {
  test_struct_decl();
  return 0;
}
