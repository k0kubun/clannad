void pass();

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

void test() {
  test_c89_comment();
  test_c99_comment();
}
