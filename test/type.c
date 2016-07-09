void expect_int(int, int);

void test_sizeof() {
  expect_int(4, sizeof(int));
  expect_int(1, sizeof(char));
}

void test() {
  test_sizeof();
}
