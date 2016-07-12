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

void test_undef() {
  int n = 0;
# define undef_test
# ifdef undef_test
  n += 1;
# endif
# undef undef_test
# ifdef undef_test
  n += 2;
# endif
# ifndef undef_test
  n += 4;
# endif
  expect_int(5, n);
}

int main() {
  test_define_replacement();
  test_empty_define();
  test_ifndef_guard();
  test_ifdef();
  test_undef();
  return 0;
}
