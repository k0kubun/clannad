#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <llvm-c/BitWriter.h>
#include "clannad.h"

struct clannad_options {
  bool dump_ast;
};

void
usage(int status)
{
  FILE *stream = status ? stderr : stdout;
  fprintf(
      stream,
      "Usage: clannad <file>\n"
      "\n"
      "  -fdump-ast  Print AST\n"
      "  -h          Print this help\n"
      "\n"
      );
  exit(status);
}

char*
parse_opts(int argc, char **argv, struct clannad_options *opts)
{
  *opts = (struct clannad_options){
    .dump_ast = false,
  };

  int opt;
  while ((opt = getopt(argc, argv, "hf:")) != -1) {
    switch (opt) {
      case 'f':
        opts->dump_ast = true;
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

int
main(int argc, char **argv)
{
  struct clannad_options opts;
  char *filename = parse_opts(argc, argv, &opts);
  FILE *file     = open_file(filename);

  Node *ast;
  if (parse_file(&ast, file)) {
    fprintf(stderr, "Error!\n");
    return 1;
  }
  fclose(file);

  if (opts.dump_ast)
    dump_ast(ast);

  LLVMModuleRef mod = compile(ast);
  optimize(mod);

  LLVMWriteBitcodeToFile(mod, "main.bc");

  return 0;
}
