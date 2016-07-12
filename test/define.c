#define foo 3

void expect_int(int, int);

void test_define_replacement() {
  expect_int(3, foo);
}

void test() {
  test_define_replacement();
}
