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
compile_float(Node *node)
{
  assert_node(node, NODE_FLOAT);
  return LLVMConstReal(LLVMFloatType(), node->fval);
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

LLVMTypeRef compile_type(Node *node);

LLVMTypeRef
compile_struct_type(Node *node)
{
  assert_node(node, NODE_TYPE);

  // Cache type if possible
  LLVMTypeRef *types = malloc(sizeof(LLVMTypeRef));
  int argc = 0;
  for (int i = 0; i < node->fields->length; i++) {
    Node *field_node = vector_get(node->fields, i);
    assert_node(field_node, NODE_FIELD);
    for (int j = 0; j < field_node->fields->length; j++) {
      types = realloc(types, (argc + 1) * sizeof(LLVMTypeRef));
      types[argc] = compile_type(field_node->field_type);
      argc++;
    }
  }
  return LLVMStructType(types, argc, false);
}

LLVMTypeRef
compile_type(Node *node)
{
  assert_node(node, NODE_TYPE);

  if (node->flags & TYPE_INT) {
    return LLVMInt32Type();
  } else if (node->flags & TYPE_CHAR) {
    return LLVMInt8Type();
  } else if (node->flags & TYPE_SHORT) {
    return LLVMInt16Type();
  } else if (node->flags & TYPE_LONG) {
    return LLVMInt32Type();
  } else if (node->flags & TYPE_FLOAT) {
    return LLVMFloatType();
  } else if (node->flags & TYPE_DOUBLE) {
    return LLVMFloatType();
  } else if (node->flags & TYPE_SIGNED) {
    return LLVMInt32Type();
  } else if (node->flags & TYPE_UNSIGNED) {
    return LLVMInt32Type();
  } else if (node->flags & TYPE_VOID) {
    return LLVMVoidType();
  } else if (node->flags & TYPE_STRUCT) {
    return compile_struct_type(node);
  } else {
    fprintf(stderr, "Unexpected type given in compile_type: %ld\n", node->flags);
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
    case ADD_ASSIGN:
      node->rhs = create_node(&(Node){ NODE_BINOP, .lhs = node->lhs, .op = '+', .rhs = node->rhs });
      node->op  = '=';
      break;
    case SUB_ASSIGN:
      node->rhs = create_node(&(Node){ NODE_BINOP, .lhs = node->lhs, .op = '-', .rhs = node->rhs });
      node->op  = '=';
      break;
    case MUL_ASSIGN:
      node->rhs = create_node(&(Node){ NODE_BINOP, .lhs = node->lhs, .op = '*', .rhs = node->rhs });
      node->op  = '=';
      break;
    case DIV_ASSIGN:
      node->rhs = create_node(&(Node){ NODE_BINOP, .lhs = node->lhs, .op = '/', .rhs = node->rhs });
      node->op  = '=';
      break;
    case MOD_ASSIGN:
      node->rhs = create_node(&(Node){ NODE_BINOP, .lhs = node->lhs, .op = '%', .rhs = node->rhs });
      node->op  = '=';
      break;
    case AND_ASSIGN:
      node->rhs = create_node(&(Node){ NODE_BINOP, .lhs = node->lhs, .op = '&', .rhs = node->rhs });
      node->op  = '=';
      break;
    case XOR_ASSIGN:
      node->rhs = create_node(&(Node){ NODE_BINOP, .lhs = node->lhs, .op = '^', .rhs = node->rhs });
      node->op  = '=';
      break;
    case OR_ASSIGN:
      node->rhs = create_node(&(Node){ NODE_BINOP, .lhs = node->lhs, .op = '|', .rhs = node->rhs });
      node->op  = '=';
      break;
  }
}

LLVMValueRef
compile_assign_dest(Node *node)
{
  switch (node->kind) {
    case NODE_IDENTIFIER:
      return node->ref_node->ref;
    case NODE_FIELD_REF:
      // FIXME: Support non-variable struct_node
      return LLVMBuildStructGEP(compiler.builder, node->struct_node->ref_node->ref, node->ref_index, "");
    case NODE_ARRAY_REF: {
      LLVMValueRef indices[2];
      indices[0] = LLVMConstInt(LLVMInt32Type(), 0, 0); // FIXME: Why is it required?
      indices[1] = compile_exp(node->rhs); // FIXME: Correct type?
      return LLVMBuildGEP(compiler.builder, node->lhs->ref_node->ref, indices, 2, "");
    }
    default:
      fprintf(stderr, "compile_assign_dest: unexpected node kind in lvalue: %s\n", kind_label(node->kind));
      exit(1);
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
      return LLVMBuildStore(compiler.builder, compile_exp(node->rhs), compile_assign_dest(node->lhs));
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
compile_ternary(Node *node)
{
  assert_node(node, NODE_TERNARY);
  return LLVMBuildSelect(compiler.builder, compile_exp(node->cond),
      compile_exp(node->if_stmt), compile_exp(node->else_stmt), "");
}

LLVMValueRef
compile_comma(Node *node)
{
  assert_node(node, NODE_COMMA);

  compile_exp(node->lhs);
  return compile_exp(node->rhs);
}

LLVMValueRef
compile_field_ref(Node *node)
{
  assert_node(node, NODE_FIELD_REF);

  // FIXME: Support non-variable struct_node
  LLVMValueRef ref = LLVMBuildStructGEP(compiler.builder, node->struct_node->ref_node->ref, node->ref_index, "");
  return LLVMBuildLoad(compiler.builder, ref, "");
}

LLVMValueRef
compile_array_ref(Node *node)
{
  assert_node(node, NODE_ARRAY_REF);

  // FIXME: Support non-variable array
  LLVMValueRef indices[2];
  indices[0] = LLVMConstInt(LLVMInt32Type(), 0, 0); // FIXME: Why is it required?
  indices[1] = compile_exp(node->rhs); // FIXME: Correct type?
  LLVMValueRef ref = LLVMBuildGEP(compiler.builder, node->lhs->ref_node->ref, indices, 2, "");
  return LLVMBuildLoad(compiler.builder, ref, "");
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
    case NODE_FLOAT:
      return compile_float(node);
    case NODE_STRING:
      return compile_string(node);
    case NODE_IDENTIFIER:
      return compile_variable(node);
    case NODE_FUNCALL:
      return compile_funcall(node);
    case NODE_TERNARY:
      return compile_ternary(node);
    case NODE_COMMA:
      return compile_comma(node);
    case NODE_FIELD_REF:
      return compile_field_ref(node);
    case NODE_ARRAY_REF:
      return compile_array_ref(node);
    default:
      fprintf(stderr, "Unexpected node in compile_exp: %s\n", kind_label(node->kind));
      exit(1);
  }
}

void
compile_var_decl(Node *node)
{
  assert_node(node, NODE_VAR_DECL);
  if (node->type->flags & TYPE_TYPEDEF) return;

  switch (node->spec->kind) {
    case NODE_SPEC:
      node->ref = LLVMBuildAlloca(compiler.builder, compile_type(node->type), "");
      if (node->init) {
        LLVMBuildStore(compiler.builder, compile_exp(node->init), node->ref);
      }
      break;
    case NODE_ARRAY_SPEC:
      assert_node(node->spec->lhs, NODE_SPEC);
      if (node->spec->rhs->kind != NODE_INTEGER) {
        fprintf(stderr, "variable length array is currently not supported\n");
        exit(1);
      }
      node->ref = LLVMBuildAlloca(compiler.builder, LLVMArrayType(compile_type(node->type), node->spec->rhs->ival), "");
      break;
    default:
      fprintf(stderr, "Unexpected spec kind in compile_var_decl: %s\n", kind_label(node->spec->kind));
      exit(1);
  }

  if (node->type->flags & TYPE_VOLATILE) LLVMSetVolatile(node->ref, true);
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

void compile_decln(Node *node);

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
    case NODE_DECLN:
      compile_decln(node);
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

  LLVMTypeRef params[node->spec->params->length];
  for (int i = 0; i < node->spec->params->length; i++) {
    params[i] = compile_param_decl((Node *)vector_get(node->spec->params, i));
  }

  LLVMAddFunction(compiler.mod, func_name(node->spec),
      LLVMFunctionType(compile_type(node->type), params, node->spec->params->length, false));
}

void
compile_decln(Node *node)
{
  assert_node(node, NODE_DECLN);

  for (int i = 0; i < node->children->length; i++) {
    Node *child = (Node *)vector_get(node->children, i);
    switch (child->kind) {
      case NODE_FUNC_DECL:
        compile_func_decl(child);
        break;
      case NODE_VAR_DECL:
        compile_var_decl(child);
        break;
      case NODE_TYPEDEF:
        break; // ignore
      default:
        fprintf(stderr, "Unexpected node kind in compile_decln: %s\n", kind_label(child->kind));
        exit(1);
    }
  }
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
      case NODE_DECLN:
        compile_decln(child);
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
