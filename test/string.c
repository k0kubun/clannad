#include "test_helper.h"

void test_newline() {
  "Hello world\n";
  pass();
}

void test_multibyte_string() {
  "クラナド";
  pass();
}

void test_escape_sequences() {
  "\a\b\f\n\r\t";
  pass();
}

void test_char() {
  char ch = 'a';
}

int main() {
  test_newline();
  test_multibyte_string();
  test_char();
  return 0;
}
