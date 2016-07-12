#include "test_helper.h"

#define foo 3
void test_define_replacement() {
  expect_int(3, foo);
}

#define empty
void test_empty_define() {
  empty;
  pass();
}

void test_ifndef_guard() {
#  include "test_helper.h"
}

#define ifdef_test
#ifdef ifdef_test
void test_ifdef() {
  pass();
}
#endif

int main() {
  test_define_replacement();
  test_empty_define();
  test_ifndef_guard();
  test_ifdef();
  return 0;
}
