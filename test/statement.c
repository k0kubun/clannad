void expect_int(int, int);
void pass();

void test_empty_comp_stmt() {
  {}
  pass();
}

void test_variable_redefinition() {
  int a;
  a = 1;
  {
    int a;
    a = 2;
    expect_int(2, a);
  }
  expect_int(1, a);
}

void test_variable_outside_comp_stmt() {
  int a;
  a = 1;
  {
    expect_int(1, a);
  }
}

void test() {
  test_empty_comp_stmt();
  test_variable_redefinition();
  test_variable_outside_comp_stmt();
}
