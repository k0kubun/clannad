void expect_int(int, int);

void test_bit_and() {
  expect_int(1, 3 & 1);
  expect_int(2, 3 & 2);
  expect_int(0, 4 & 2);
}

void test_bit_or() {
  expect_int(3, 3 | 1);
  expect_int(3, 3 | 2);
  expect_int(6, 4 | 2);
}

void test_bit_xor() {
  expect_int(2, 3 ^ 1);
  expect_int(1, 3 ^ 2);
  expect_int(6, 4 ^ 2);
}

void test_bit_not() {
  int a;
  a = -4;
  expect_int(3, ~a);
  expect_int(-3, ~2);
}

void test_left_shift() {
  expect_int(8, 2 << 2);
}

void test_left_assign() {
  int n;
  n = 2;
  n <<= 2;
  expect_int(8, n);
}

void test_right_shift() {
  expect_int(2, 8 >> 2);
}

void test_right_assign() {
  int n;
  n = 8;
  n >>= 2;
  expect_int(2, n);
}

void test() {
  test_bit_and();
  test_bit_or();
  test_bit_xor();
  test_bit_not();
  test_left_shift();
  test_left_assign();
  test_right_shift();
  test_right_assign();
}
