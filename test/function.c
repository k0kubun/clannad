void expect_int(int a, int b);
void pass();

int ret_func() { return 3; }
int var_ret_func() {
  int n;
  n = 3;
  return n;
}

void test_return() {
  expect_int(3, ret_func());
  expect_int(3, var_ret_func());
  return;
}

void test_void_return() {
  pass();
  return;
}

int sum_func(int a, int b, int c) { return a + b + c; }

void test_funcall() {
  expect_int(10, sum_func(2, 3, 5));
  return;
}

void test_void_decl(void);

void test_void_arg(void) {
  pass();
  return;
}

void test() {
  test_return();
  test_void_return();
  test_funcall();
  test_void_arg();
  return;
}
