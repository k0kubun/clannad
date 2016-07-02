#include <llvm-c/Core.h>
#include <llvm-c/BitWriter.h>

void
assemble(LLVMModuleRef mod, char *outfile)
{
  LLVMWriteBitcodeToFile(mod, "main.bc");
}
