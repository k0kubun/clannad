int printf(char *__format);

int main() {
  int a;
  a = 1;
  a--;
  if (a)
    printf("yes\n");
  else
    printf("no\n");
  return 0;
}
