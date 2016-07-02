int printf(char *__format);
int test();

int expect_int(int a, int b) {
  if (a == b)
    printf(".");
  else
    printf("F\nexpected %d but got %d\n", a, b);
  return 0;
}

int main() {
  test();
  return 0;
}
