#include "test_helper.h"

void test_sizeof() {
  expect_int(4, sizeof(int));
  expect_int(1, sizeof(char));
}

int main() {
  test_sizeof();
  return 0;
}
