int printf(char *__format, int a);

int twice(int a) {
  int ret;
  ret = a * 2;
  return ret;
}

int main() {
  printf("The result is: %d\n", twice(10));
  return 0;
}
