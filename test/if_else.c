#include "test_helper.h"

int deep_else(int flag1, int flag2) {
  int n = 0;
  if (flag1)
    if (flag2)
      n = 1;
    else
      n = 2;
  return n;
}

int shallow_else(int flag1, int flag2) {
  int n = 0;
  if (flag1)
    if (flag2)
      n = 1;
  else
    n = 2;
  return n;
}

void test_dangling_else() {
  expect_int(0, deep_else(0, 0));
  expect_int(0, deep_else(0, 1));
  expect_int(0, shallow_else(0, 0));
  expect_int(0, shallow_else(0, 1));

  expect_int(2, deep_else(1, 0));
  expect_int(1, deep_else(1, 1));
  expect_int(2, shallow_else(1, 0));
  expect_int(1, shallow_else(1, 1));
}

void test_if_without_block() {
  int a = 1;
  if (1)
    a = a * 2;
  else
    a = 0;
  expect_int(2, a);
}

void test_if_with_block() {
  int a = 1;
  if (1) {
    a = a * 2;
    a = a + 1;
  } else {
    a = 0;
  }
  expect_int(3, a);

  if (0) {
    a = 0;
  } else {
    a = a * 2;
  }
  expect_int(6, a);
}

void test_comma() {
  int a;
  if (1,0) a = 1;
  else a = 2;
  expect_int(2, a);

  if (0,1) a = 1;
  else a = 2;
  expect_int(1, a);

  if (a = 2, a - 1) a = 3;
  else a = 4;
  expect_int(3, a);
}

void test() {
  test_dangling_else();
  test_if_without_block();
  test_if_with_block();
  test_comma();
}
