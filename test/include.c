#include "test/include.h"

void test_included_func_call() {
  pass();
}

void test() {
  test_included_func_call();
}
