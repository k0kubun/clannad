int expect_int(int a, int b);
int printf(char *__format);

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

int test() {
  test_return();
  test_funcall();
  return 0;
}
