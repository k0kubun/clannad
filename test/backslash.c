#in\
clude "test_\
helper.h"

#def\
ine TE\
N 10

vo\
id test_removing_backslash\
_newline\
() {
  expect_int(T\
EN, 1\
0);
}

i\
nt ma\
in() {
  test_removing_backslash_newline(\
);
  ret\
urn 0;
}
