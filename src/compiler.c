#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "clannad.h"

void
assert_node(Node *node, enum NodeType type)
{
  if (node->type != type) {
    fprintf(stderr, "InternalError: node type assertion failed!\n");
    fprintf(stderr, "  expected '%s' but got '%s'\n", type_label(type), type_label(node->type));
    exit(1);
  }
}

LLVMValueRef
compile_int(Node *node)
{
  assert_node(node, NODE_INTEGER);
  return LLVMConstInt(LLVMInt32Type(), node->ival, false);
}

LLVMValueRef
compile_string(LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_STRING);

  // FIXME: Handle escape sequence in node->id
  return LLVMBuildGlobalStringPtr(builder, node->id, "");
}

void
compile_return(LLVMModuleRef mod, LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_RETURN);

  switch (node->param->type) {
    case NODE_INTEGER:
      LLVMBuildRet(builder, compile_int(node->param));
      break;
    default:
      fprintf(stderr, "Unexpected node type in compile_return: %s\n", type_label(node->param->type));
      exit(1);
  }
}

void
compile_funcall(LLVMModuleRef mod, LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_FUNCALL);

  LLVMValueRef args[256]; // FIXME: Handle array limit properly
  LLVMValueRef func = LLVMGetNamedFunction(mod, node->func->id);

  // Build arguments
  for (int i = 0; i < node->params->length; i++) {
    Node *param = (Node *)vector_get(node->params, i);
    switch (param->type) {
      case NODE_STRING:
        args[i] = compile_string(builder, param);
        break;
      default:
        fprintf(stderr, "Unexpected node type in compile_funcall: %s\n", type_label(param->type));
        exit(1);
    }
  }

  LLVMBuildCall(builder, func, args, node->params->length, "");
}

void
compile_stmt(LLVMModuleRef mod, LLVMBasicBlockRef block, Node *node)
{
  assert_node(node, NODE_COMPOUND_STMT);

  // build block instructions
  LLVMBuilderRef builder = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(builder, block);

  for (int i = 0; i < node->children->length; i++) {
    Node *child = (Node *)vector_get(node->children, i);
    switch (child->type) {
      case NODE_FUNCALL:
        compile_funcall(mod, builder, child);
        break;
      case NODE_RETURN:
        compile_return(mod, builder, child);
        break;
      default:
        fprintf(stderr, "Unexpected node type in compile_stmt: %s\n", type_label(child->type));
        exit(1);
    }
  }
}

char*
func_name(Node *node)
{
  assert_node(node, NODE_FUNC_SPEC);
  assert_node(node->func, NODE_SPEC);
  return node->func->id;
}

void
compile_func(LLVMModuleRef mod, Node *node)
{
  assert_node(node, NODE_FUNC);

  // declare function
  char *func = func_name(node->decl);
  LLVMTypeRef params[] = { LLVMInt32Type(), LLVMInt32Type() };
  LLVMValueRef main_func = LLVMAddFunction(mod, func, LLVMFunctionType(LLVMInt32Type(), params, 2, false));

  // create block for function
  char block_name[256]; // FIXME: dynamic allocation
  sprintf(block_name, "%s_block", func);
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
        fprintf(stderr, "Unexpected node type in compile_root: %s\n", type_label(child->type));
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
