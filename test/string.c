void pass();

void test_newline() {
  "Hello world\n";
  pass();
  return;
}

void test_multibyte_string() {
  "クラナド";
  pass();
  return;
}

void test_char() {
  char ch;
  ch = 'a';
  return;
}

void test() {
  test_newline();
  test_multibyte_string();
  test_char();
  return;
}
