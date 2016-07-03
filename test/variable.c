void pass();

void test_variable() {
  int n;
  n = 10;
  n = n + 3;
  pass();
  return;
}

void test() {
  test_variable();
  return;
}
