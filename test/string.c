void pass();

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

void test() {
  test_newline();
  test_multibyte_string();
  test_char();
}
