void expect_int(int a, int b);

void test_arithmetic() {
  expect_int(3, 1 + 2);
  expect_int(5, 7 - 2);
  expect_int(6, 2 * 3);
  expect_int(3, 9 / 3);
  expect_int(2, 8 % 3);
  return;
}

void test_not() {
  int n;
  if (!1)
    n = 1;
  else
    n = 2;
  expect_int(2, n);

  if (!n)
    n = 3;
  else
    n = 4;
  expect_int(4, n);
  return;
}

void test() {
  test_arithmetic();
  test_not();
  return;
}
