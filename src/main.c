#include <stdbool.h>
#include <stdio.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include "clannad.h"

void
hello_world()
{
  LLVMModuleRef mod = LLVMModuleCreateWithName("clannad");

  // declare printf function
  LLVMTypeRef printf_params[] = { LLVMPointerType(LLVMInt8Type(), false) };
  LLVMValueRef printf_func = LLVMAddFunction(mod, "printf", LLVMFunctionType(LLVMInt32Type(), printf_params, 1, false));

  // declare main function
  LLVMTypeRef main_params[] = { LLVMInt32Type(), LLVMInt32Type() };
  LLVMValueRef main_func = LLVMAddFunction(mod, "main", LLVMFunctionType(LLVMInt32Type(), main_params, 2, false));

  // create block for main
  LLVMBasicBlockRef block = LLVMAppendBasicBlock(main_func, "main_block");

  // build block instructions
  LLVMBuilderRef builder = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(builder, block);

  // printf("Hello World!\n")
  LLVMValueRef args[] = { LLVMBuildGlobalStringPtr(builder, "Hello world!\n", "") };
  LLVMBuildCall(builder, printf_func, args, 1, "");

  // return 0
  LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, false));

  LLVMWriteBitcodeToFile(mod, "main.bc");
}

int
main(void)
{
  Node *ast;

  if (parse_stdin(&ast)) {
    fprintf(stderr, "Error!\n");
    return 1;
  }
  dump_ast(ast);

  hello_world();
  return 0;
}