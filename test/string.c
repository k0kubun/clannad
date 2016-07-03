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

void test() {
  test_newline();
  test_multibyte_string();
  return;
}
