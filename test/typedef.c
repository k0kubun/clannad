#include "test_helper.h"

typedef int INT1;

void test_typedef() {
  INT1 a = 1;
  expect_int(1, a);
}

int main() {
  test_typedef();
  return 0;
}
