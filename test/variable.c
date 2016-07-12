#include "test_helper.h"

void test_variable() {
  int n;
  n = 10;
  n = n + 3;
  expect_int(13, n);
}

void test_multiple_declarations() {
  int a, b;
  a = 2;
  b = 3;
  expect_int(5, a + b);
}

void test_initialized_variable() {
  int n = 3;
  expect_int(3, n);
}

void test_const_variable() {
  const int a = 3;
  const const int b = 4;
  expect_int(3, a);
  expect_int(4, b);
}

void test_volatile_variable() {
  volatile int a = 3;
  a++;
  expect_int(4, a);
}

void test() {
  test_variable();
  test_multiple_declarations();
  test_initialized_variable();
  test_const_variable();
}
