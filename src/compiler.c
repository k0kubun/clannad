#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

LLVMValueRef compile_exp(LLVMBuilderRef builder, Node *node);

void
compile_return(LLVMModuleRef mod, LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_RETURN);
  LLVMBuildRet(builder, compile_exp(builder, node->param));
}

LLVMValueRef
compile_binop(LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_BINOP);
  switch (node->op) {
    case '+':
      return LLVMBuildAdd(builder, compile_exp(builder, node->lhs), compile_exp(builder, node->rhs), "");
    case '-':
      return LLVMBuildSub(builder, compile_exp(builder, node->lhs), compile_exp(builder, node->rhs), "");
    case '*':
      return LLVMBuildMul(builder, compile_exp(builder, node->lhs), compile_exp(builder, node->rhs), "");
    case '/':
      return LLVMBuildSDiv(builder, compile_exp(builder, node->lhs), compile_exp(builder, node->rhs), "");
    case '=':
      // FIXME: Use LLVMBuildStore
      return compile_exp(builder, node->rhs);
    default:
      fprintf(stderr, "Unexpected binary operation: %c\n", node->op);
      exit(1);
  }
}

LLVMValueRef
compile_exp(LLVMBuilderRef builder, Node *node)
{
  switch (node->type) {
    case NODE_BINOP:
      return compile_binop(builder, node);
    case NODE_INTEGER:
      return compile_int(node);
    case NODE_STRING:
      return compile_string(builder, node);
    default:
      fprintf(stderr, "Unexpected node in compile_exp: %s\n", type_label(node->type));
      exit(1);
  }
}

LLVMValueRef
compile_funcall(LLVMModuleRef mod, LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_FUNCALL);

  LLVMValueRef args[256]; // FIXME: Handle array limit properly
  LLVMValueRef func = LLVMGetNamedFunction(mod, node->func->id);

  // Build arguments
  for (int i = 0; i < node->params->length; i++) {
    Node *param = (Node *)vector_get(node->params, i);
    if (param->type == NODE_FUNCALL) {
      // FIXME: compile_exp should have compile_funcall
      args[i] = compile_funcall(mod, builder, param);
    } else {
      args[i] = compile_exp(builder, param);
    }
  }

  return LLVMBuildCall(builder, func, args, node->params->length, "");
}

LLVMTypeRef
compile_type(Node *node)
{
  assert_node(node, NODE_TYPE);

  // FIXME: We should have this as token
  if (strcmp(node->id, "int") == 0) {
    return LLVMInt32Type();
  } else if (strcmp(node->id, "char") == 0) {
    return LLVMInt8Type();
  } else {
    fprintf(stderr, "Unexpected id in compile_type: %s\n", node->id);
    exit(1);
  }
}

void
compile_var_decl(LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_VAR_DECL);

  LLVMBuildAlloca(builder, compile_type(node->spec), node->decl->id);
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
      case NODE_VAR_DECL:
        compile_var_decl(builder, child);
        break;
      case NODE_BINOP:
        compile_binop(builder, child);
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

LLVMTypeRef
compile_param_decl(Node *node)
{
  assert_node(node, NODE_PARAM_DECL);

  if (node->decl->type == NODE_PTR) {
    return LLVMPointerType(compile_type(node->spec), false);
  } else {
    return compile_type(node->spec);
  }
}

void
compile_func(LLVMModuleRef mod, Node *node)
{
  assert_node(node, NODE_FUNC);

  // declare function
  char *func = func_name(node->decl);
  LLVMTypeRef params[256]; // FIXME: dynamic allocation
  for (int i = 0; i < node->decl->params->length; i++) {
    params[i] = compile_param_decl((Node *)vector_get(node->decl->params, i));
  }
  LLVMValueRef main_func = LLVMAddFunction(mod, func,
      LLVMFunctionType(LLVMInt32Type(), params, node->decl->params->length, false));

  // create block for function
  char block_name[256]; // FIXME: dynamic allocation
  sprintf(block_name, "%s_block", func);
  LLVMBasicBlockRef block = LLVMAppendBasicBlock(main_func, block_name);

  compile_stmt(mod, block, node->stmts);
}

void
compile_func_decl(LLVMModuleRef mod, Node *node)
{
  assert_node(node, NODE_FUNC_DECL);

  LLVMTypeRef params[256]; // FIXME: dynamic allocation
  for (int i = 0; i < node->decl->params->length; i++) {
    params[i] = compile_param_decl((Node *)vector_get(node->decl->params, i));
  }

  LLVMAddFunction(mod, func_name(node->decl),
      LLVMFunctionType(compile_type(node->spec), params, node->decl->params->length, false));
}

void
compile_root(LLVMModuleRef mod, Node *node)
{
  assert_node(node, NODE_ROOT);

  for (int i = 0; i < node->children->length; i++) {
    Node *child = (Node *)vector_get(node->children, i);
    switch (child->type) {
      case NODE_FUNC:
        compile_func(mod, child);
        break;
      case NODE_FUNC_DECL:
        compile_func_decl(mod, child);
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
