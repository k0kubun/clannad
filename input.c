int printf(char *__format, int n1, int n2);

int factorial(int n) {
  int ret;
  if (n == 1)
    ret = 1;
  else
    ret = n * factorial(n - 1);
  return ret;
}

int main() {
  int num;
  num = 5;
  printf("factorial(%d) = %d\n", num, factorial(num));
  return 0;
}
