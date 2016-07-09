#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "clannad.h"

typedef struct {
  LLVMModuleRef mod;
  LLVMValueRef func;
  LLVMBuilderRef builder;
} Compiler;

static Compiler compiler;

LLVMValueRef
compile_int(Node *node)
{
  assert_node(node, NODE_INTEGER);
  return LLVMConstInt(LLVMInt32Type(), node->ival, false);
}

LLVMValueRef
compile_string(Node *node)
{
  assert_node(node, NODE_STRING);
  return LLVMBuildGlobalStringPtr(compiler.builder, node->id, "");
}

LLVMValueRef
compile_variable(Node *node)
{
  assert_node(node, NODE_IDENTIFIER);
  return LLVMBuildLoad(compiler.builder, node->ref_node->ref, "");
}

LLVMValueRef compile_exp(Node *node);

void
compile_return(Node *node)
{
  assert_node(node, NODE_RETURN);

  if (node->param) {
    LLVMBuildRet(compiler.builder, compile_exp(node->param));
  } else {
    LLVMBuildRetVoid(compiler.builder);
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

LLVMValueRef
compile_unary(Node *node)
{
  assert_node(node, NODE_UNARY);

  if (node->op == SIZEOF)
    return LLVMConstIntCast(LLVMSizeOf(compile_type(node->val)), LLVMInt32Type(), 0);

  LLVMValueRef var, result = compile_exp(node->val);
  switch ((int)node->op) {
    case PRE_INC_OP:
      var = node->val->ref_node->ref;
      LLVMBuildStore(compiler.builder, LLVMBuildAdd(compiler.builder, result, LLVMConstInt(LLVMInt32Type(), 1, 0), ""), var);
      return LLVMBuildLoad(compiler.builder, var, "");
    case PRE_DEC_OP:
      var = node->val->ref_node->ref;
      LLVMBuildStore(compiler.builder, LLVMBuildSub(compiler.builder, result, LLVMConstInt(LLVMInt32Type(), 1, 0), ""), var);
      return LLVMBuildLoad(compiler.builder, var, "");
    case POST_INC_OP:
      var = node->val->ref_node->ref;
      LLVMBuildStore(compiler.builder, LLVMBuildAdd(compiler.builder, result, LLVMConstInt(LLVMInt32Type(), 1, 0), ""), var);
      return result;
    case POST_DEC_OP:
      var = node->val->ref_node->ref;
      LLVMBuildStore(compiler.builder, LLVMBuildSub(compiler.builder, result, LLVMConstInt(LLVMInt32Type(), 1, 0), ""), var);
      return result;
    case '+':
      return result;
    case '-':
      return LLVMBuildNeg(compiler.builder, result, "");
    case '~':
      return LLVMBuildNot(compiler.builder, result, "");
    case '!':
      return LLVMBuildICmp(compiler.builder, LLVMIntEQ, result, LLVMConstInt(LLVMTypeOf(result), 0, 0), "");
    default:
      fprintf(stderr, "Unexpected operator in compile_unary: %d\n", (int)node->op);
      exit(1);
  }
}

void
convert_compound_assign(Node *node)
{
  switch (node->op) {
    case RIGHT_ASSIGN:
      node->rhs = create_node(&(Node){ NODE_BINOP, .lhs = node->lhs, .op = RIGHT_OP, .rhs = node->rhs });
      node->op  = '=';
      break;
    case LEFT_ASSIGN:
      node->rhs = create_node(&(Node){ NODE_BINOP, .lhs = node->lhs, .op = LEFT_OP, .rhs = node->rhs });
      node->op  = '=';
      break;
  }
}

LLVMValueRef
compile_binop(Node *node)
{
  assert_node(node, NODE_BINOP);

  convert_compound_assign(node);
  LLVMValueRef var;
  switch (node->op) {
    case '+':
      return LLVMBuildAdd(compiler.builder, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case '-':
      return LLVMBuildSub(compiler.builder, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case '*':
      return LLVMBuildMul(compiler.builder, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case '/':
      return LLVMBuildSDiv(compiler.builder, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case '%':
      return LLVMBuildSRem(compiler.builder, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case '=':
      return LLVMBuildStore(compiler.builder, compile_exp(node->rhs), node->lhs->ref_node->ref);
    case '<':
      return LLVMBuildICmp(compiler.builder, LLVMIntSLT, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case '>':
      return LLVMBuildICmp(compiler.builder, LLVMIntSGT, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case '&':
      return LLVMBuildAnd(compiler.builder, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case '|':
      return LLVMBuildOr(compiler.builder, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case '^':
      return LLVMBuildXor(compiler.builder, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case LE_OP:
      return LLVMBuildICmp(compiler.builder, LLVMIntSLE, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case GE_OP:
      return LLVMBuildICmp(compiler.builder, LLVMIntSGE, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case EQ_OP:
      return LLVMBuildICmp(compiler.builder, LLVMIntEQ, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case NE_OP:
      return LLVMBuildICmp(compiler.builder, LLVMIntNE, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case AND_OP:
      return LLVMBuildBinOp(compiler.builder, LLVMAnd, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case OR_OP:
      return LLVMBuildBinOp(compiler.builder, LLVMOr, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case LEFT_OP:
      return LLVMBuildBinOp(compiler.builder, LLVMShl, compile_exp(node->lhs), compile_exp(node->rhs), "");
    case RIGHT_OP:
      return LLVMBuildBinOp(compiler.builder, LLVMLShr, compile_exp(node->lhs), compile_exp(node->rhs), "");
    default:
      fprintf(stderr, "Unexpected binary operation: %c\n", node->op);
      exit(1);
  }
}

LLVMValueRef
compile_funcall(Node *node)
{
  assert_node(node, NODE_FUNCALL);

  LLVMValueRef args[node->params->length];
  LLVMValueRef func = LLVMGetNamedFunction(compiler.mod, node->func->id);

  // Build arguments
  for (int i = 0; i < node->params->length; i++) {
    Node *param = vector_get(node->params, i);
    args[i] = compile_exp(param);
  }

  return LLVMBuildCall(compiler.builder, func, args, node->params->length, "");
}

LLVMValueRef
compile_exp(Node *node)
{
  switch (node->kind) {
    case NODE_UNARY:
      return compile_unary(node);
    case NODE_BINOP:
      return compile_binop(node);
    case NODE_INTEGER:
      return compile_int(node);
    case NODE_STRING:
      return compile_string(node);
    case NODE_IDENTIFIER:
      return compile_variable(node);
    case NODE_FUNCALL:
      return compile_funcall(node);
    default:
      fprintf(stderr, "Unexpected node in compile_exp: %s\n", kind_label(node->kind));
      exit(1);
  }
}

void
compile_var_decl(Node *node)
{
  assert_node(node, NODE_VAR_DECL);
  node->ref = LLVMBuildAlloca(compiler.builder, compile_type(node->type), node->spec->id);
}

void compile_stmt(Node *node);

void
compile_if(Node *node)
{
  assert_node(node, NODE_IF);

  LLVMBasicBlockRef if_block   = LLVMAppendBasicBlock(compiler.func, "if");
  LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(compiler.func, "else");
  LLVMBasicBlockRef end_block  = LLVMAppendBasicBlock(compiler.func, "end");

  LLVMValueRef cond_exp = compile_exp(node->cond);
  LLVMValueRef cond = LLVMBuildICmp(compiler.builder, LLVMIntNE,
      cond_exp, LLVMConstInt(LLVMTypeOf(cond_exp), 0, 0), "");
  LLVMBuildCondBr(compiler.builder, cond, if_block, else_block);

  LLVMPositionBuilderAtEnd(compiler.builder, if_block);
  compile_stmt(node->if_stmt);
  LLVMBuildBr(compiler.builder, end_block);

  LLVMPositionBuilderAtEnd(compiler.builder, else_block);
  if (node->else_stmt) compile_stmt(node->else_stmt);
  LLVMBuildBr(compiler.builder, end_block);

  LLVMPositionBuilderAtEnd(compiler.builder, end_block);
}

void
compile_comp_stmt(Node *node)
{
  assert_node(node, NODE_COMPOUND_STMT);

  for (int i = 0; i < node->children->length; i++) {
    Node *child = (Node *)vector_get(node->children, i);
    compile_stmt(child);
  }
}

void
compile_stmt(Node *node)
{
  switch (node->kind) {
    case NODE_BINOP:
    case NODE_INTEGER:
    case NODE_STRING:
    case NODE_IDENTIFIER:
    case NODE_FUNCALL:
    case NODE_UNARY:
      compile_exp(node);
      break;
    case NODE_COMPOUND_STMT:
      compile_comp_stmt(node);
      break;
    case NODE_IF:
      compile_if(node);
      break;
    case NODE_RETURN:
      compile_return(node);
      break;
    case NODE_VAR_DECL:
      compile_var_decl(node);
      break;
    default:
      fprintf(stderr, "Unexpected node kind in compile_stmt: %s\n", kind_label(node->kind));
      exit(1);
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

  if (node->spec && node->spec->kind == NODE_PTR) {
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
  LLVMTypeRef param_types[node->spec->params->length];
  for (int i = 0; i < node->spec->params->length; i++) {
    param_types[i] = compile_param_decl((Node *)vector_get(node->spec->params, i));
  }
  compiler.func = LLVMAddFunction(compiler.mod, func_name(node->spec),
      LLVMFunctionType(compile_type(node->type), param_types, node->spec->params->length, false));

  // build block instructions
  LLVMBasicBlockRef block = LLVMAppendBasicBlock(compiler.func, "entry");
  compiler.builder = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(compiler.builder, block);

  // set and store argument names
  LLVMValueRef param_refs[node->spec->params->length];
  LLVMGetParams(compiler.func, param_refs);
  for (int i = 0; i < node->spec->params->length; i++) {
    Node *param = vector_get(node->spec->params, i);
    param->ref = LLVMBuildAlloca(compiler.builder, compile_type(param->type), param->spec->id);
    LLVMBuildStore(compiler.builder, param_refs[i], param->ref);
  }

  // Avoiding compile_comp_stmt since we have extra variables (argument variables) for this block
  assert_node(node->stmts, NODE_COMPOUND_STMT);
  for (int i = 0; i < node->stmts->children->length; i++) {
    Node *child = (Node *)vector_get(node->stmts->children, i);
    compile_stmt(child);
  }
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
    .mod = LLVMModuleCreateWithName("clannad"),
  };
  compile_root(ast);
  return compiler.mod;
}
