void expect_int(int a, int b);
void pass();

void test_variable() {
  int n;
  n = 10;
  n = n + 3;
  pass();
}

void test_multiple_declarations() {
  int a, b;
  a = 2;
  b = 3;
  expect_int(5, a + b);
}

void test() {
  test_variable();
  test_multiple_declarations();
}
