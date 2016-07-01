#include <stdbool.h>
#include <stdio.h>
#include <llvm-c/BitWriter.h>
#include <stdlib.h>
#include <unistd.h>
#include "clannad.h"

void
usage(int status)
{
  FILE *stream = status ? stderr : stdout;
  fprintf(
      stream,
      "Usage: clannad [-j] <file>\n"
      "\n"
      "  -j  Execute file with JIT\n"
      "  -h  Print this help\n"
      "\n"
      );
  exit(status);
}

char*
parseopt(int argc, char **argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "jh")) != -1) {
    switch (opt) {
      case 'j':
        printf("JIT\n");
        break;
      case 'h':
        usage(0);
      default:
        usage(1);
    }
  }

  if (optind != argc - 1)
    usage(1);
  return argv[optind];
}

int
main(int argc, char **argv)
{
  Node *ast;
  char *filename;

  filename = parseopt(argc, argv);
  if (parse_stdin(&ast)) {
    fprintf(stderr, "Error!\n");
    return 1;
  }

  LLVMModuleRef mod = compile(ast);
  optimize(mod);

  LLVMWriteBitcodeToFile(mod, "main.bc");

  return 0;
}
