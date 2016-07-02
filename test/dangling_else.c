int expect_int(int a, int b);

int deep_else(int flag1, int flag2) {
  int n;
  n = 0;
  if (flag1)
    if (flag2)
      n = 1;
    else
      n = 2;
  return n;
}

int shallow_else(int flag1, int flag2) {
  int n;
  n = 0;
  if (flag1)
    if (flag2)
      n = 1;
  else
    n = 2;
  return n;
}

int test() {
  expect_int(0, deep_else(0, 0));
  expect_int(0, deep_else(0, 1));
  expect_int(0, shallow_else(0, 0));
  expect_int(0, shallow_else(0, 1));

  expect_int(2, deep_else(1, 0));
  expect_int(1, deep_else(1, 1));
  expect_int(2, shallow_else(1, 0));
  expect_int(1, shallow_else(1, 1));

  return 0;
}
