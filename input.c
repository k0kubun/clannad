int printf(char *__format, int a);

int foo() {
  printf("The result is: %d\n", 5 + 4 * 3 - 2 / 1);
  return 0;
}

int main() {
  foo();
  return 0;
}
