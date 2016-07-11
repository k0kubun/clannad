#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <llvm-c/BitWriter.h>
#include "clannad.h"

struct clannad_options {
  bool dump_ast;
  bool dump_ir1;
  bool dump_ir2;
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
      "  -fdump-ir1  Print LLVM IR before optimization\n"
      "  -fdump-ir2  Print LLVM IR after optimization\n"
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
    .dump_ir1 = false,
    .dump_ir2 = false,
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
        if (!strcmp(optarg, "dump-ast")) {
          opts->dump_ast = true;
        } else if (!strcmp(optarg, "dump-ir1")) {
          opts->dump_ir1 = true;
        } else if (!strcmp(optarg, "dump-ir2")) {
          opts->dump_ir2 = true;
        } else {
          fprintf(stderr, "Unknown option: -f%s\n\n", optarg);
          usage(1);
        }
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

int
main(int argc, char **argv)
{
  struct clannad_options opts;
  char *filename = parse_opts(argc, argv, &opts);

  Node *ast;
  if (parse_file(&ast, filename)) {
    fprintf(stderr, "Error!\n");
    return 1;
  }

  if (opts.dump_ast) dump_ast(ast);
  analyze(ast);

  LLVMModuleRef mod = compile(ast);
  if (opts.dump_ir1) LLVMDumpModule(mod);

  optimize(mod);
  if (opts.dump_ir2) LLVMDumpModule(mod);

  assemble(mod, opts.outfile);
  return 0;
}
