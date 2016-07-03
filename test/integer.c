void expect_int(int a, int b);

void test_arithmetic() {
  expect_int(3, 1 + 2);
  expect_int(5, 7 - 2);
  expect_int(6, 2 * 3);
  expect_int(3, 9 / 3);
  expect_int(2, 8 % 3);
  return;
}

void test() {
  test_arithmetic();
  return;
}
