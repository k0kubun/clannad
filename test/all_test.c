int expect_int(int a, int b);
int printf(char *__format);

int test_arithmetic() {
  expect_int(3, 1 + 2);
  expect_int(5, 7 - 2);
  expect_int(6, 2 * 3);
  expect_int(3, 9 / 3);
  return 0;
}

int ret_func() { return 3; }

int test_return() {
  expect_int(3, ret_func());
  return 0;
}

int sum_func(int a, int b, int c) { return a + b + c; }

int test_funcall() {
  expect_int(10, sum_func(2, 3, 5));
  return 0;
}

int test_newline() {
  "Hello world\n";
  printf(".");
  return 0;
}

int test_v() {
  int n;
  printf(".");
  return 0;
}

int test() {
  test_arithmetic();
  test_return();
  test_funcall();
  test_newline();
  test_v();
  return 0;
}
