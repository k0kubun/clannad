void expect_int(int a, int b);

void test_bit_and() {
  expect_int(1, 3 & 1);
  expect_int(2, 3 & 2);
  expect_int(0, 4 & 2);
  return;
}

void test_bit_or() {
  expect_int(3, 3 | 1);
  expect_int(3, 3 | 2);
  expect_int(6, 4 | 2);
  return;
}

void test_bit_xor() {
  expect_int(2, 3 ^ 1);
  expect_int(1, 3 ^ 2);
  expect_int(6, 4 ^ 2);
  return;
}

void test_left_shift() {
  expect_int(8, 2 << 2);
  return;
}

void test_right_shift() {
  expect_int(2, 8 >> 2);
  return;
}

void test() {
  test_bit_and();
  test_bit_or();
  test_bit_xor();
  test_left_shift();
  test_right_shift();
  return;
}
