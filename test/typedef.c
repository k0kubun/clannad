#include "test_helper.h"

typedef int INT1;

void test_typedef() {
  INT1 a = 1;
  expect_int(1, a);
}

void test_voidptr_typedef() {
  typedef void* voidptr;
  pass();
}

void test_struct_typedef() {
  typedef struct {
    int a;
  } b;
  b c;
  c.a = 3;
  expect_int(3, c.a);
}

int main() {
  test_typedef();
  test_voidptr_typedef();
  test_struct_typedef();
  return 0;
}
