void expect_int(int a, int b);

void test_sizeof() {
  expect_int(4, sizeof(int));
  expect_int(1, sizeof(char));
  return;
}

void test() {
  test_sizeof();
  return;
}
