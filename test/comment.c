#include "test_helper.h"
#include "test_helper.h"

/* comment */
void test_c89_comment() {
  /* comment */
  pass();
}

/*===-- header -*-===*\
|*                   *|
|*===-------------===*|
|*                   *|
\*===-------------===*/

/*
int test() { exit(1); }
 */

// comment
void test_c99_comment() {
  // comment
  pass();
}

// int test() { exit(1); }

void test_comment_should_not_rewind_state() {
# if 0
  /* */
  expect_int(1, 0);
# endif
}

int main() {
  test_c89_comment();
  test_c99_comment();
  test_comment_should_not_rewind_state();
  return 0;
}
