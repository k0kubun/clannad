#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "clannad.h"

// Compile NODE_RETURN
void
compile_return(LLVMModuleRef mod, LLVMBuilderRef builder, Node *node)
{
  // return 0
  LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, false));
}

// Compile NODE_FUNCALL
void
compile_funcall(LLVMModuleRef mod, LLVMBuilderRef builder, Node *node)
{
  // printf("Hello World!\n")
  LLVMValueRef args[] = { LLVMBuildGlobalStringPtr(builder, "Hello world!\n", "") };
  LLVMValueRef printf_func = LLVMGetNamedFunction(mod, "printf");
  LLVMBuildCall(builder, printf_func, args, 1, "");
}

// Compile NODE_COMPOUND_STMT
void
compile_stmt(LLVMModuleRef mod, LLVMBasicBlockRef block, Node *node)
{
  // build block instructions
  LLVMBuilderRef builder = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(builder, block);

  for (int i = 0; i < node->children->length; i++) {
    // TODO: Switch by node type
    Node *child = (Node *)vector_get(node->children, i);
    switch (child->type) {
      case NODE_FUNCALL:
        compile_funcall(mod, builder, child);
        break;
      case NODE_RETURN:
        compile_return(mod, builder, child);
        break;
      default:
        fprintf(stderr, "Unexpected node type in compile_stmt: %d\n", child->type);
        exit(1);
    }
  }
}

// Compile NODE_FUNC
void
compile_func(LLVMModuleRef mod, Node *node)
{
  // TODO: Assert node type

  // declare main function
  LLVMTypeRef main_params[] = { LLVMInt32Type(), LLVMInt32Type() };
  LLVMValueRef main_func = LLVMAddFunction(mod, "main", LLVMFunctionType(LLVMInt32Type(), main_params, 2, false));

  // create block for main
  LLVMBasicBlockRef block = LLVMAppendBasicBlock(main_func, "main_block");
  compile_stmt(mod, block, node->stmts);
}

// Compile NODE_ROOT
void
compile_root(LLVMModuleRef mod, Node *node)
{
  // TODO: Assert node type

  // TODO: Compile declaration properly
  // declare printf function
  LLVMTypeRef printf_params[] = { LLVMPointerType(LLVMInt8Type(), false) };
  LLVMAddFunction(mod, "printf", LLVMFunctionType(LLVMInt32Type(), printf_params, 1, false));

  for (int i = 0; i < node->children->length; i++) {
    // TODO: Switch by node type
    compile_func(mod, (Node *)vector_get(node->children, i));
  }
}

LLVMModuleRef
compile(Node *ast)
{
  LLVMModuleRef mod = LLVMModuleCreateWithName("clannad");
  compile_root(mod, ast);
  return mod;
}
