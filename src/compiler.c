#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "clannad.h"

void
assert_node(Node *node, enum NodeType type)
{
  if (node->type != type) {
    fprintf(stderr, "InternalError: node type assertion failed!\n");
    fprintf(stderr, "  expected %d but got %d\n", type, node->type);
    exit(1);
  }
}

void
compile_return(LLVMModuleRef mod, LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_RETURN);

  // return 0
  LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, false));
}

void
compile_funcall(LLVMModuleRef mod, LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_FUNCALL);

  // printf("Hello World!\n")
  LLVMValueRef args[] = { LLVMBuildGlobalStringPtr(builder, "Hello world!\n", "") };
  LLVMValueRef printf_func = LLVMGetNamedFunction(mod, "printf");
  LLVMBuildCall(builder, printf_func, args, 1, "");
}

void
compile_stmt(LLVMModuleRef mod, LLVMBasicBlockRef block, Node *node)
{
  assert_node(node, NODE_COMPOUND_STMT);

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

void
compile_func(LLVMModuleRef mod, Node *node)
{
  assert_node(node, NODE_FUNC);

  // declare function
  char *func_name = node->decl->id;
  LLVMTypeRef params[] = { LLVMInt32Type(), LLVMInt32Type() };
  LLVMValueRef main_func = LLVMAddFunction(mod, func_name, LLVMFunctionType(LLVMInt32Type(), params, 2, false));

  // create block for function
  char block_name[256]; // FIXME: dynamic allocation
  sprintf(block_name, "%s_block", func_name);
  LLVMBasicBlockRef block = LLVMAppendBasicBlock(main_func, block_name);

  compile_stmt(mod, block, node->stmts);
}

void
compile_root(LLVMModuleRef mod, Node *node)
{
  assert_node(node, NODE_ROOT);

  // TODO: Compile declaration properly
  // declare printf function
  LLVMTypeRef printf_params[] = { LLVMPointerType(LLVMInt8Type(), false) };
  LLVMAddFunction(mod, "printf", LLVMFunctionType(LLVMInt32Type(), printf_params, 1, false));

  for (int i = 0; i < node->children->length; i++) {
    Node *child = (Node *)vector_get(node->children, i);
    switch (child->type) {
      case NODE_FUNC:
        compile_func(mod, child);
        break;
      default:
        fprintf(stderr, "Unexpected node type in compile_root: %d\n", child->type);
        exit(1);
    }
  }
}

LLVMModuleRef
compile(Node *ast)
{
  LLVMModuleRef mod = LLVMModuleCreateWithName("clannad");
  compile_root(mod, ast);
  return mod;
}
