void test_line_num() {
# line 2
}

void test_line_file() {
# line 6 "test/line.c"
}

int main() {
  test_line_num();
  test_line_file();
  return 0;
}
