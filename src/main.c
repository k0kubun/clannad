#include <stdbool.h>
#include <stdio.h>
#include <llvm-c/BitWriter.h>
#include "clannad.h"

int
main(void)
{
  Node *ast;

  if (parse_stdin(&ast)) {
    fprintf(stderr, "Error!\n");
    return 1;
  }

  LLVMModuleRef mod = compile(ast);
  optimize(mod);

  LLVMWriteBitcodeToFile(mod, "main.bc");

  return 0;
}
