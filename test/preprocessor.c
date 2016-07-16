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

void test_elif() {
  int n = 0;
# if defined defined_val
# elif defined defined_val
  n += 1;
# endif
# if defined undefined_val
# elif defined defined_val
  n += 2;
# endif
  expect_int(2, n);
}

void test_defined_var() {
  int n = 0;
# define A 1
# if defined A && A
  n += 1;
# endif
# if defined undefined_val && undefined_val
  n += 2;
# endif
  expect_int(1, n);
}

void test_nested_ifdef() {
  int n = 0;
# ifdef undefined_val
  n += 1;
#   ifdef undefined_val
  n += 2;
#   endif
# endif

# ifndef defined_val
  n += 1;
#   ifndef defined_val
  n += 2;
#   endif
# endif
  expect_int(0, n);
}

void test_nested_if() {
  int n = 0;
# if defined undefined_val
  n += 1;
#   if (undefined_val - 0) >= 500
  n += 2;
#   endif
# endif
  expect_int(0, n);
}

void test_if_arithmetic() {
  int n = 0;
# if (1 + 2) / 3 >= 1
  n++;
# endif
  expect_int(1, n);
}

#define FUNC(a, b) a) * (b
void test_func_macro() {
  int a = (1 + FUNC(1,2) + 3);
  expect_int(10, a);
}

#define GT1(a) a > 1
void test_func_macro_in_pp() {
  int n = 0;
# if GT1(2)
  n += 1;
# endif
# if GT1(1)
  n += 2;
# endif
  expect_int(1, n);
}

void test_else() {
  int n = 0;
# if 0
  n += 1;
# else
  n += 2;
# endif
  expect_int(2, n);
}

void test_comment_elif_ignored() {
# if defined undefined_val
/*
# elif undefined_val
*/
# endif
  pass();
}

void test_else_with_comment() {
# if 1
# else /* comment */
# endif
  pass();
}

void test_predefined_macros() {
  expect_int(4, __GNUC__);
  expect_int(1, __STDC__);
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
  test_elif();
  test_defined_var();
  test_nested_ifdef();
  test_nested_if();
  test_if_arithmetic();
  test_func_macro();
  test_func_macro_in_pp();
  test_else();
  test_comment_elif_ignored();
  test_else_with_comment();
  return 0;
}
