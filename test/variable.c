void expect_int(int a, int b);

void test_variable() {
  int n;
  n = 10;
  n = n + 3;
  expect_int(13, n);
}

void test_multiple_declarations() {
  int a, b;
  a = 2;
  b = 3;
  expect_int(5, a + b);
}

void test_initialized_variable() {
  int n = 3;
  expect_int(3, n);
}

void test() {
  test_variable();
  test_multiple_declarations();
  test_initialized_variable();
}
