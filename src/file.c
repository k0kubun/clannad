#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE*
open_file(char *filename)
{
  if (!strcmp(filename, "-"))
    return stdin;

  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Failed to open: '%s'\n", filename);
    exit(1);
  }
  return file;
}

char*
read_fp(FILE *fp)
{
  fseek(fp, 0L, SEEK_END);
  long size = ftell(fp);
  rewind(fp);

  char *ret = malloc(size * sizeof(char));
  fread(ret, sizeof(char), size, fp);
  fclose(fp);

  return ret;
}

char*
scan_backslash_newline(char *str)
{
  // FIXME: This function should keep line numbers...
  char *ret = malloc((strlen(str)+1) * sizeof(char));
  char prev1 = '\0', prev2 = '\0';
  int i, j;
  for (i = 0, j = 0; i < strlen(str); i++, j++) {
    if (str[i] == '\n' && prev1 == '\\' && prev2 != '\\') {
      j -= 2;
    } else {
      ret[j] = str[i];
    }
    prev2 = prev1;
    prev1 = str[i];
  }
  free(str);
  ret[j] = '\0';
  return ret;
}

FILE*
drop_backslash_newline(FILE *fp)
{
  char *str = scan_backslash_newline(read_fp(fp));
  FILE *ret = fmemopen(str, strlen(str), "r");
  if (!ret) {
    fprintf(stderr, "Failed to fmemopen: '%s'\n", str);
    exit(1);
  }
  return ret;
}
