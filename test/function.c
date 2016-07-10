void expect_int(int a, int b);
void pass();

int ret_func() { return 3; }
int var_ret_func() {
  int n = 3;
  return n;
}

void test_return() {
  expect_int(3, ret_func());
  expect_int(3, var_ret_func());
}

void test_void_return() {
  pass();
}

void test_implicit_return() {
  pass();
}

int sum_func(int a, int b, int c) { return a + b + c; }

void test_funcall() {
  expect_int(10, sum_func(2, 3, 5));
}

void test_void_decl(void);

void test_pointer_decl_with_name(char *s1, char *s2);
void test_pointer_decl_without_name(char *, char *);

void test_void_arg(void) {
  pass();
}

void test() {
  test_return();
  test_void_return();
  test_funcall();
  test_void_arg();
}
