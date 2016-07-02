#include <stdio.h>
#include <llvm-c/Core.h>
#include <llvm-c/BitWriter.h>

void
assemble(LLVMModuleRef mod, char *outfile)
{
  LLVMWriteBitcodeToFile(mod, "main.bc");

  FILE *llcin = popen("llc", "w");
  LLVMWriteBitcodeToFD(mod, fileno(llcin), 1, 0);
}
