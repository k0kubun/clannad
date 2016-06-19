int printf(char *__format, int a);

int foo() {
  int num;
  num = 5 + 4 * 3 - 2 / 1;
  return num;
}

int main() {
  printf("The result is: %d\n", foo());
  return 0;
}
