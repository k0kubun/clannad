void pass();

void test_variable() {
  int n;
  n = 10;
  n = n + 3;
  pass();
}

void test() {
  test_variable();
}
