int printf(char *__format, int a);

int foo() {
  return 5 + 4 * 3 - 2 / 1;
}

int main() {
  int num;
  printf("The result is: %d\n", foo());
  return 0;
}
