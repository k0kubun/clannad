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
# include "test_helper.h"
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

# define defined_val
void test_nested_ifdef() {
  int n = 0;
# ifdef defined_val
  n += 1;
#   ifdef undefined_val
  n += 2;
#   endif
  n += 4;
# endif
  expect_int(5, n);
}

void test_nested_if_without_parenthesis() {
  int n = 0;
# if defined defined_val
  n += 1;
#   if defined undefined_val
  n += 2;
#   endif
  n += 4;
# endif
  expect_int(5, n);
}

void test_nested_if_with_parenthesis() {
  int n = 0;
# if defined(defined_val)
  n += 1;
#   if defined(undefined_val)
  n += 2;
#   endif
  n += 4;
# endif
  expect_int(5, n);
}

void test_if_expression() {
  int n = 0;
# if (defined(defined_val) && !defined undefined_val)
  n = 1;
# endif
  expect_int(1, n);
}

void test_multiline_if() {
  int n = 0;
# if (defined(defined_val) && \
    !defined undefined_val)
  n = 1;
# endif
  expect_int(1, n);
}

int main() {
  test_define_replacement();
  test_empty_define();
  test_ifndef_guard();
  test_ifdef();
  test_undef();
  test_nested_ifdef();
  test_nested_if_without_parenthesis();
  test_nested_if_with_parenthesis();
  test_if_expression();
  test_multiline_if();
  return 0;
}
