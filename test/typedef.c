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

int main() {
  test_typedef();
  test_voidptr_typedef();
  return 0;
}
