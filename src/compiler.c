#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "clannad.h"

typedef struct {
  LLVMModuleRef mod;
  LLVMValueRef func;
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
  if (node->param) {
    LLVMBuildRet(builder, compile_exp(builder, node->param));
  } else {
    LLVMBuildRetVoid(builder);
  }
}

LLVMValueRef
compile_unary(LLVMBuilderRef builder, Node *node)
{
  Node *target;
  if (node->lhs) target = node->lhs;
  if (node->rhs) target = node->rhs;

  LLVMValueRef var;
  LLVMValueRef result = compile_exp(builder, target);
  switch ((int)node->op) {
    case INC_OP:
      // FIXME: maybe some assertion required
      var = dict_get(compiler.syms, target->id);
      LLVMBuildStore(builder, LLVMBuildAdd(builder, result, LLVMConstInt(LLVMInt32Type(), 1, 0), ""), var);
      break;
    case DEC_OP:
      // FIXME: maybe some assertion required
      var = dict_get(compiler.syms, target->id);
      LLVMBuildStore(builder, LLVMBuildSub(builder, result, LLVMConstInt(LLVMInt32Type(), 1, 0), ""), var);
      break;
    default:
      fprintf(stderr, "Unexpected operator in compile_unary: %d\n", (int)node->op);
      exit(1);
  }
  // TODO: check whether result is correct or not to return
  return result;
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
    case EQ_OP:
      return LLVMBuildICmp(builder, LLVMIntEQ, compile_exp(builder, node->lhs), compile_exp(builder, node->rhs), "");
    case NE_OP:
      return LLVMBuildICmp(builder, LLVMIntNE, compile_exp(builder, node->lhs), compile_exp(builder, node->rhs), "");
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
    case NODE_UNARY:
      return compile_unary(builder, node);
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

  if (!strcmp(node->id, "int")) {
    return LLVMInt32Type();
  } else if (!strcmp(node->id, "char")) {
    return LLVMInt8Type();
  } else if (!strcmp(node->id, "void")) {
    return LLVMVoidType();
  } else {
    fprintf(stderr, "Unexpected id in compile_type: %s\n", node->id);
    exit(1);
  }
}

void
compile_var_decl(LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_VAR_DECL);

  LLVMValueRef var = LLVMBuildAlloca(builder, compile_type(node->type), node->spec->id);
  dict_set(compiler.syms, node->spec->id, var);
}

void compile_stmt(LLVMBuilderRef builder, Node *node);

void
compile_if(LLVMBuilderRef builder, Node *node)
{
  LLVMBasicBlockRef if_block   = LLVMAppendBasicBlock(compiler.func, "if");
  LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(compiler.func, "else");
  LLVMBasicBlockRef end_block  = LLVMAppendBasicBlock(compiler.func, "end");

  LLVMValueRef cond_exp = compile_exp(builder, node->cond);
  LLVMValueRef cond = LLVMBuildICmp(builder, LLVMIntNE,
      cond_exp, LLVMConstInt(LLVMTypeOf(cond_exp), 0, 0), "");
  LLVMBuildCondBr(builder, cond, if_block, else_block);

  LLVMPositionBuilderAtEnd(builder, if_block);
  compile_stmt(builder, node->if_stmt);
  LLVMBuildBr(builder, end_block);

  LLVMPositionBuilderAtEnd(builder, else_block);
  if (node->else_stmt) compile_stmt(builder, node->else_stmt);
  LLVMBuildBr(builder, end_block);

  LLVMPositionBuilderAtEnd(builder, end_block);
}

void
compile_stmt(LLVMBuilderRef builder, Node *node)
{
  switch (node->kind) {
    case NODE_BINOP:
    case NODE_INTEGER:
    case NODE_STRING:
    case NODE_IDENTIFIER:
    case NODE_FUNCALL:
    case NODE_UNARY:
      compile_exp(builder, node);
      break;
    case NODE_VAR_DECL:
      compile_var_decl(builder, node);
      break;
    case NODE_IF:
      compile_if(builder, node);
      break;
    case NODE_RETURN:
      compile_return(builder, node);
      break;
    default:
      fprintf(stderr, "Unexpected node kind in compile_stmt: %s\n", kind_label(node->kind));
      exit(1);
  }
}

void
compile_comp_stmt(LLVMBuilderRef builder, Node *node)
{
  assert_node(node, NODE_COMPOUND_STMT);

  for (int i = 0; i < node->children->length; i++) {
    Node *child = (Node *)vector_get(node->children, i);
    compile_stmt(builder, child);
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

  if (node->spec->kind == NODE_PTR) {
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
  LLVMTypeRef param_types[256]; // FIXME: dynamic allocation
  for (int i = 0; i < node->spec->params->length; i++) {
    param_types[i] = compile_param_decl((Node *)vector_get(node->spec->params, i));
  }
  compiler.func = LLVMAddFunction(compiler.mod, func_name(node->spec),
      LLVMFunctionType(compile_type(node->type), param_types, node->spec->params->length, false));

  // Create local scope
  compiler.syms = create_dict();

  // set argument names
  LLVMValueRef params[256]; // FIXME: dynamic allocation
  LLVMGetParams(compiler.func, params);
  for (int i = 0; i < node->spec->params->length; i++) {
    char *param_name = ((Node *)vector_get(node->spec->params, i))->spec->id;
    dict_set(compiler.syms, param_name, params[i]);
    LLVMSetValueName(params[i], param_name);
  }

  // build block instructions
  LLVMBasicBlockRef block = LLVMAppendBasicBlock(compiler.func, "entry");
  LLVMBuilderRef builder = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(builder, block);

  // store argument
  for (int i = 0; i < node->spec->params->length; i++) {
    Node *param = vector_get(node->spec->params, i);
    LLVMValueRef val = dict_get(compiler.syms, param->spec->id);
    LLVMValueRef var = LLVMBuildAlloca(builder, compile_type(param->type), param->spec->id);
    dict_set(compiler.syms, param->spec->id, var);
    LLVMBuildStore(builder, val, var);
  }

  compile_comp_stmt(builder, node->stmts);
}

void
compile_func_decl(Node *node)
{
  assert_node(node, NODE_FUNC_DECL);

  LLVMTypeRef params[256]; // FIXME: dynamic allocation
  for (int i = 0; i < node->spec->params->length; i++) {
    params[i] = compile_param_decl((Node *)vector_get(node->spec->params, i));
  }

  LLVMAddFunction(compiler.mod, func_name(node->spec),
      LLVMFunctionType(compile_type(node->type), params, node->spec->params->length, false));
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
