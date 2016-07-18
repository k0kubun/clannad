#include "test_helper.h"

void test_line_num() {
# line 4
  pass();
}

void test_line_file() {
# line 9 "test/line.c"
  pass();
}

int main() {
  test_line_num();
  test_line_file();
  return 0;
}
