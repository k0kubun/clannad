void pass();

/* comment */
void test_c89_comment() {
  /* comment */
  pass();
  return;
}

/*
int test() { exit(1); }
 */

void test() {
  test_c89_comment();
  return;
}
