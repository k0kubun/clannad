void expect_int(int, int);

void test_equal() {
  int n = 1;
  if (n == 1)
    n = 2;
  else
    n = 3;
  expect_int(2, n);
}

void test_gt() {
  int n = 1;
  if (n > 0)
    n = 2;
  expect_int(2, n);

  if (n > 2)
    n = 3;
  expect_int(2, n);
}

void test_lt() {
  int n = 1;
  if (n < 2)
    n = 2;
  expect_int(2, n);

  if (n < 2)
    n = 3;
  expect_int(2, n);
}

void test_ge() {
  int n = 1;
  if (n >= 0)
    n = 2;
  expect_int(2, n);

  if (n >= 2)
    n = 3;
  expect_int(3, n);
}

void test_le() {
  int n = 1;
  if (n <= 2)
    n = 2;
  expect_int(2, n);

  if (n <= 2)
    n = 3;
  expect_int(3, n);
}

void test_and_op() {
  expect_int(1, 2 && 2);
  expect_int(0, 2 && 0);
  expect_int(0, 0 && 0);
}

void test_or_op() {
  expect_int(1, 2 || 2);
  expect_int(1, 2 || 0);
  expect_int(0, 0 || 0);
}

void test_cond_op() {
  expect_int(2, 1 ? 2 : 3);
  expect_int(3, 0 ? 2 : 3);
}

void test() {
  test_equal();
  test_gt();
  test_lt();
  test_ge();
  test_le();
  test_cond_op();
}
