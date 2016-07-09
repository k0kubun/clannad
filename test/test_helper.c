int printf(char *, int, int);
void test();

void pass() {
  printf(".", 0, 0);
}

void expect_int(int a, int b) {
  if (a == b)
    pass();
  else
    printf("F\nexpected %d but got %d\n", a, b);
}

int main() {
  test();
  return 0;
}
