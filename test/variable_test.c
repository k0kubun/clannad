int printf(char *__format);

int test_variable() {
  int n;
  n = 10;
  n = n + 3;
  printf(".");
  return 0;
}

int test() {
  test_variable();
  return 0;
}
