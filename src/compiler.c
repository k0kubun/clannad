#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "clannad.h"

typedef struct {
  LLVMModuleRef mod;
  Dict *syms; /* Symbol table for compiling scope */
} Compiler;

static Compiler compiler;

void
assert_node(Node *node, enum NodeKind kind)
{
  if (node->kind != kind) {
    fprintf(stderr, "InternalError: node kind assertion failed!\n");
    fprintf(stderr, "  expected '%s' but got '%s'\n", kind_label(kind), kind_label(node->kind));
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
  return LLVMBuildGlobalStringPtr(builder, node->id, "");
}

LLVMValueRef
compile_variable(LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_IDENTIFIER);

  LLVMValueRef var = dict_get(compiler.syms, node->id);
  if (var == NULL) {
    fprintf(stderr, "Undefined variable: %s\n", node->id);
    exit(1);
  }
  return LLVMBuildLoad(builder, var, node->id);
}

LLVMValueRef compile_exp(LLVMBuilderRef builder, Node *node);

void
compile_return(LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_RETURN);
  LLVMBuildRet(builder, compile_exp(builder, node->param));
}

LLVMValueRef
compile_binop(LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_BINOP);

  LLVMValueRef var;
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
      var = dict_get(compiler.syms, node->lhs->id);
      if (var == NULL) {
        fprintf(stderr, "Undefined variable: %s\n", node->lhs->id);
        exit(1);
      }
      return LLVMBuildStore(builder, compile_exp(builder, node->rhs), var);
    default:
      fprintf(stderr, "Unexpected binary operation: %c\n", node->op);
      exit(1);
  }
}

LLVMValueRef
compile_funcall(LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_FUNCALL);

  LLVMValueRef args[256]; // FIXME: Handle array limit properly
  LLVMValueRef func = LLVMGetNamedFunction(compiler.mod, node->func->id);

  // Build arguments
  for (int i = 0; i < node->params->length; i++) {
    Node *param = (Node *)vector_get(node->params, i);
    args[i] = compile_exp(builder, param);
  }

  return LLVMBuildCall(builder, func, args, node->params->length, "");
}

LLVMValueRef
compile_exp(LLVMBuilderRef builder, Node *node)
{
  switch (node->kind) {
    case NODE_BINOP:
      return compile_binop(builder, node);
    case NODE_INTEGER:
      return compile_int(node);
    case NODE_STRING:
      return compile_string(builder, node);
    case NODE_IDENTIFIER:
      return compile_variable(builder, node);
    case NODE_FUNCALL:
      return compile_funcall(builder, node);
    default:
      fprintf(stderr, "Unexpected node in compile_exp: %s\n", kind_label(node->kind));
      exit(1);
  }
}

LLVMTypeRef
compile_type(Node *node)
{
  assert_node(node, NODE_TYPE);

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

  LLVMValueRef var = LLVMBuildAlloca(builder, compile_type(node->type), node->decl->id);
  dict_set(compiler.syms, node->decl->id, var);
}

void
compile_stmt(LLVMBasicBlockRef block, Node *node)
{
  assert_node(node, NODE_COMPOUND_STMT);

  // Create local scope
  compiler.syms = create_dict();

  // build block instructions
  LLVMBuilderRef builder = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(builder, block);

  for (int i = 0; i < node->children->length; i++) {
    Node *child = (Node *)vector_get(node->children, i);
    switch (child->kind) {
      case NODE_BINOP:
      case NODE_INTEGER:
      case NODE_STRING:
      case NODE_IDENTIFIER:
      case NODE_FUNCALL:
        compile_exp(builder, child);
        break;
      case NODE_VAR_DECL:
        compile_var_decl(builder, child);
        break;
      case NODE_RETURN:
        compile_return(builder, child);
        break;
      default:
        fprintf(stderr, "Unexpected node kind in compile_stmt: %s\n", kind_label(child->kind));
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

  if (node->decl->kind == NODE_PTR) {
    return LLVMPointerType(compile_type(node->type), false);
  } else {
    return compile_type(node->type);
  }
}

void
compile_func(Node *node)
{
  assert_node(node, NODE_FUNC);

  // declare function
  char *func = func_name(node->decl);
  LLVMTypeRef params[256]; // FIXME: dynamic allocation
  for (int i = 0; i < node->decl->params->length; i++) {
    params[i] = compile_param_decl((Node *)vector_get(node->decl->params, i));
  }
  LLVMValueRef main_func = LLVMAddFunction(compiler.mod, func,
      LLVMFunctionType(compile_type(node->type), params, node->decl->params->length, false));

  // create block for function
  char block_name[256]; // FIXME: dynamic allocation
  sprintf(block_name, "%s_block", func);
  LLVMBasicBlockRef block = LLVMAppendBasicBlock(main_func, block_name);

  compile_stmt(block, node->stmts);
}

void
compile_func_decl(Node *node)
{
  assert_node(node, NODE_FUNC_DECL);

  LLVMTypeRef params[256]; // FIXME: dynamic allocation
  for (int i = 0; i < node->decl->params->length; i++) {
    params[i] = compile_param_decl((Node *)vector_get(node->decl->params, i));
  }

  LLVMAddFunction(compiler.mod, func_name(node->decl),
      LLVMFunctionType(compile_type(node->type), params, node->decl->params->length, false));
}

void
compile_root(Node *node)
{
  assert_node(node, NODE_ROOT);

  for (int i = 0; i < node->children->length; i++) {
    Node *child = (Node *)vector_get(node->children, i);
    switch (child->kind) {
      case NODE_FUNC:
        compile_func(child);
        break;
      case NODE_FUNC_DECL:
        compile_func_decl(child);
        break;
      default:
        fprintf(stderr, "Unexpected node kind in compile_root: %s\n", kind_label(child->kind));
        exit(1);
    }
  }
}

LLVMModuleRef
compile(Node *ast)
{
  compiler = (Compiler){
    .mod  = LLVMModuleCreateWithName("clannad"),
    .syms = create_dict(),
  };
  compile_root(ast);
  return compiler.mod;
}
