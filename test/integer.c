void expect_int(int a, int b);

void test_arithmetic() {
  expect_int(3, 1 + 2);
  expect_int(5, 7 - 2);
  expect_int(6, 2 * 3);
  expect_int(3, 9 / 3);
  expect_int(2, 8 % 3);
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
}

int factorial(int n) {
  int ret;
  if (n == 1)
    ret = 1;
  else
    ret = n * factorial(n - 1);
  return ret;
}

void test_factorial() {
  expect_int(120, factorial(5));
}

void test_increment() {
  int n;

  n = 0;
  expect_int(0, n++);
  expect_int(1, n);

  n = 0;
  expect_int(1, ++n);
  expect_int(1, n);
}

void test_decrement() {
  int n;

  n = 0;
  expect_int(0, n--);
  expect_int(-1, n);

  n = 0;
  expect_int(-1, --n);
  expect_int(-1, n);
}

void test() {
  test_arithmetic();
  test_not();
  test_factorial();
  test_increment();
  test_decrement();
}
