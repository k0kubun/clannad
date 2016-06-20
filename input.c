int printf(char *__format, int a);

int foo(int a) {
  int ret;
  ret = 5 + 4 * 3 - 2 / 1;
  return ret;
}

int main() {
  printf("The result is: %d\n", foo(3));
  return 0;
}
