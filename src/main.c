#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <llvm-c/BitWriter.h>
#include "clannad.h"

struct clannad_options {
  bool dump_ast;
  char *outfile;
};

void
usage(int status)
{
  FILE *stream = status ? stderr : stdout;
  fprintf(
      stream,
      "Usage: clannad [-o outfile] <file>\n"
      "\n"
      "  -o outfile  Place output in outfile\n"
      "  -fdump-ast  Print AST\n"
      "  -c          Do not link (default)\n"
      "  -h          Print this help\n"
      "\n"
      );
  exit(status);
}

char*
default_outfile(char *filename)
{
  if (!strcmp(filename, "-"))
    return "a.o";

  int len = strlen(filename);
  if (filename[len-2] == '.' && filename[len-1] == 'c') {
    char *out = (char *)malloc(sizeof(char) * (len+1));
    strcpy(out, filename);
    out[len-1] = 'o';
    return out;
  }

  char *out = (char *)malloc(sizeof(char) * (len+3));
  strcpy(out, filename);
  strcat(out, ".o");
  return out;
}

char*
parse_opts(int argc, char **argv, struct clannad_options *opts)
{
  *opts = (struct clannad_options){
    .dump_ast = false,
    .outfile  = NULL,
  };

  int opt;
  while ((opt = getopt(argc, argv, "chf:o:")) != -1) {
    switch (opt) {
      case 'c':
        break; // ignored since linker is not integrated yet
      case 'o':
        opts->outfile = optarg;
        break;
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
  char *filename = argv[optind];

  if (!opts->outfile)
    opts->outfile = default_outfile(filename);

  return filename;
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

  if (opts.dump_ast) dump_ast(ast);
  analyze(ast);

  LLVMModuleRef mod = compile(ast);
  optimize(mod);

  assemble(mod, opts.outfile);
  return 0;
}
