#include <llvm-c/Core.h>
#include <llvm-c/Transforms/Scalar.h>

LLVMPassManagerRef build_pass_manager() {
  LLVMPassManagerRef pass = LLVMCreatePassManager();

  LLVMAddPromoteMemoryToRegisterPass(pass);
  return pass;
}

void optimize(LLVMModuleRef mod) {
  LLVMPassManagerRef pass = build_pass_manager();

  LLVMRunPassManager(pass, mod);
  LLVMDisposePassManager(pass);
}
