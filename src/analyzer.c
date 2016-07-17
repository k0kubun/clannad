#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clannad.h"

typedef struct {
  Dict *scope;
} Analyzer;

static Analyzer analyzer;

void
assert_node(Node *node, enum NodeKind kind)
{
  if (node->kind != kind) {
    fprintf(stderr, "InternalError: node kind assertion failed!\n");
    fprintf(stderr, "  expected '%s' but got '%s'\n", kind_label(kind), kind_label(node->kind));
    exit(1);
  }
}

void
assert_const(Node *node, char *operation, char *var_name)
{
  assert_node(node, NODE_TYPE);

  if (node->flags & TYPE_CONST) {
    fprintf(stderr, "error: %s of read-only variable '%s'\n", operation, var_name);
    exit(1);
  }
}

void
scope_enter()
{
  Dict *new_scope = create_dict();
  new_scope->parent = analyzer.scope;
  analyzer.scope = new_scope;
}

void
scope_leave()
{
  if (!analyzer.scope->parent) {
    fprintf(stderr, "InternalError: Leaving empty scope stack!\n");
    exit(1);
  }
  // FIXME: Release memory of left dict
  analyzer.scope = analyzer.scope->parent;
}

void analyze_exp(Node *node);

void
analyze_unary(Node *node)
{
  assert_node(node, NODE_UNARY);

  if (node->op == SIZEOF) return;
  analyze_exp(node->val);

  // FIXME: reject arithmetic unary op for non-integers
  switch ((int)node->op) {
    case PRE_INC_OP:
    case PRE_DEC_OP:
    case POST_INC_OP:
    case POST_DEC_OP:
      if (node->val->ref_node->kind != NODE_VAR_DECL) {
        fprintf(stderr, "lvalue required as increment operand\n");
        exit(1);
      }
      assert_const(node->val->ref_node->type, "increment", node->val->id);
      break;
  }
}

void analyze_field_ref(Node *node);

void
analyze_binop(Node *node)
{
  assert_node(node, NODE_BINOP);

  analyze_exp(node->lhs);
  analyze_exp(node->rhs);

  switch (node->op) {
    case '=':
      switch (node->lhs->kind) {
        case NODE_IDENTIFIER:
          if (node->lhs->ref_node->kind != NODE_VAR_DECL) {
            fprintf(stderr, "lvalue required as left operand of assignment\n");
            exit(1);
          }
          assert_const(node->lhs->ref_node->type, "assignment", node->lhs->id);
          break;
        case NODE_FIELD_REF:
          // FIXME: something may be required
          break;
        default:
          fprintf(stderr, "unexpected node kind in lvalue of '=': %s\n", kind_label(node->lhs->kind));
          exit(1);
      }
      break;
  }
}

void
analyze_int(Node *node)
{
  assert_node(node, NODE_INTEGER);
}

void
analyze_float(Node *node)
{
  assert_node(node, NODE_FLOAT);
}

void
analyze_string(Node *node)
{
  assert_node(node, NODE_STRING);
}

void
analyze_variable(Node *node)
{
  assert_node(node, NODE_IDENTIFIER);

  Node *var_node = dict_get(analyzer.scope, node->id);
  if (var_node == NULL) {
    fprintf(stderr, "Undefined variable: %s\n", node->id);
    exit(1);
  }
  node->ref_node = var_node;
}

void
analyze_funcall(Node *node)
{
  assert_node(node, NODE_FUNCALL);

  for (int i = 0; i < node->params->length; i++) {
    Node *param = vector_get(node->params, i);
    analyze_exp(param);
  }
}

void
analyze_ternary(Node *node)
{
  assert_node(node, NODE_TERNARY);

  analyze_exp(node->cond);
  analyze_exp(node->if_stmt);
  analyze_exp(node->else_stmt);
}

void
analyze_comma(Node *node)
{
  assert_node(node, NODE_COMMA);

  analyze_exp(node->lhs);
  analyze_exp(node->rhs);
}

long
find_field_index(Node *struct_type, char *name)
{
  assert_node(struct_type, NODE_TYPE);

  long ref_index = 0;
  for (int i = 0; i < struct_type->fields->length; i++) {
    Node *field_node = vector_get(struct_type->fields, i);
    assert_node(field_node, NODE_FIELD);
    for (int j = 0; j < field_node->fields->length; j++) {
      Node *field_var = vector_get(field_node->fields, j);
      assert_node(field_var, NODE_SPEC);
      if (!strcmp(field_var->id, name)) return ref_index;
      ref_index++;
    }
  }
  fprintf(stderr, "Undefined field name: '%s'\n", name);
  exit(1);
}

void
analyze_field_ref(Node *node)
{
  assert_node(node, NODE_FIELD_REF);

  // FIXME: something may be required
  analyze_exp(node->struct_node);

  // FIXME: non-variable is not considered
  Node *struct_type = node->struct_node->ref_node->type;
  node->ref_index = find_field_index(struct_type, node->id);
}

void
analyze_exp(Node *node)
{
  switch (node->kind) {
    case NODE_UNARY:
      return analyze_unary(node);
    case NODE_BINOP:
      return analyze_binop(node);
    case NODE_INTEGER:
      return analyze_int(node);
    case NODE_FLOAT:
      return analyze_float(node);
    case NODE_STRING:
      return analyze_string(node);
    case NODE_IDENTIFIER:
      return analyze_variable(node);
    case NODE_FUNCALL:
      return analyze_funcall(node);
    case NODE_TERNARY:
      return analyze_ternary(node);
    case NODE_COMMA:
      return analyze_comma(node);
    case NODE_FIELD_REF:
      return analyze_field_ref(node);
    default:
      fprintf(stderr, "Unexpected node in analyze_exp: %s\n", kind_label(node->kind));
      exit(1);
  }
}

void analyze_stmt(Node *node);

void
analyze_if(Node *node)
{
  assert_node(node, NODE_IF);

  analyze_exp(node->cond);
  analyze_stmt(node->if_stmt);
  if (node->else_stmt) analyze_stmt(node->else_stmt);
}

void
analyze_return(Node *node)
{
  assert_node(node, NODE_RETURN);
  if (node->param) analyze_exp(node->param);
}

void
analyze_var_decl(Node *node)
{
  assert_node(node, NODE_VAR_DECL);
  if (node->type->flags & TYPE_TYPEDEF) return;

  dict_set(analyzer.scope, node->spec->id, node);
  if (node->init) analyze_exp(node->init);
}

void
analyze_comp_stmt(Node *node)
{
  assert_node(node, NODE_COMPOUND_STMT);

  scope_enter();
  for (int i = 0; i < node->children->length; i++) {
    Node *child = (Node *)vector_get(node->children, i);
    analyze_stmt(child);
  }
  scope_leave();
}

void analyze_decln(Node *node);

void
analyze_stmt(Node *node)
{
  switch (node->kind) {
    case NODE_BINOP:
    case NODE_INTEGER:
    case NODE_STRING:
    case NODE_IDENTIFIER:
    case NODE_FUNCALL:
    case NODE_UNARY:
      analyze_exp(node);
      break;
    case NODE_COMPOUND_STMT:
      analyze_comp_stmt(node);
      break;
    case NODE_IF:
      analyze_if(node);
      break;
    case NODE_RETURN:
      analyze_return(node);
      break;
    case NODE_DECLN:
      analyze_decln(node);
      break;
    default:
      fprintf(stderr, "Unexpected node kind in analyze_stmt: %s\n", kind_label(node->kind));
      exit(1);
  }
}

void
strip_single_void(Node *node)
{
  assert_node(node, NODE_FUNC_SPEC);

  if (node->params->length == 1) {
    Node *first = vector_get(node->params, 0);
    if ((first->type->flags & TYPE_VOID) && !first->spec) {
      // FIXME: free old vector
      node->params = create_vector();
    }
  }
}

void
autocomplete_return(Node *node)
{
  assert_node(node, NODE_COMPOUND_STMT);

  Node *last = vector_last(node->children);
  if (node->children->length == 0 ||
      ((Node *)vector_last(node->children))->kind != NODE_RETURN) {
    Node *ret_node = create_node(&(Node){ NODE_RETURN, .param = NULL });
    vector_push(node->children, ret_node);
  }
}

void
analyze_func(Node *node)
{
  assert_node(node, NODE_FUNC);

  strip_single_void(node->spec);
  scope_enter();

  for (int i = 0; i < node->spec->params->length; i++) {
    Node *param = vector_get(node->spec->params, i);
    dict_set(analyzer.scope, param->spec->id, param);
  }

  // Avoiding compile_comp_stmt since scopes of argument variables
  // and function's compound statement should be the same.
  assert_node(node->stmts, NODE_COMPOUND_STMT);
  for (int i = 0; i < node->stmts->children->length; i++) {
    Node *child = (Node *)vector_get(node->stmts->children, i);
    analyze_stmt(child);
  }

  scope_leave();
  if (node->type->flags & TYPE_VOID) {
    autocomplete_return(node->stmts);
  }
}

void
analyze_func_decl(Node *node)
{
  assert_node(node, NODE_FUNC_DECL);

  strip_single_void(node->spec);
  assert_node(node, NODE_FUNC_DECL);
}

void
analyze_decln(Node *node)
{
  assert_node(node, NODE_DECLN);

  for (int i = 0; i < node->children->length; i++) {
    Node *child = (Node *)vector_get(node->children, i);
    switch (child->kind) {
      case NODE_FUNC_DECL:
        analyze_func_decl(child);
        break;
      case NODE_VAR_DECL:
        analyze_var_decl(child);
        break;
      case NODE_TYPEDEF:
        break; // ignore
      default:
        fprintf(stderr, "Unexpected node kind in analyze_decln: %s\n", kind_label(child->kind));
        exit(1);
    }
  }
}

void
analyze_root(Node *node)
{
  assert_node(node, NODE_ROOT);

  for (int i = 0; i < node->children->length; i++) {
    Node *child = (Node *)vector_get(node->children, i);
    switch (child->kind) {
      case NODE_FUNC:
        analyze_func(child);
        break;
      case NODE_DECLN:
        analyze_decln(child);
        break;
      default:
        fprintf(stderr, "Unexpected node kind in analyze_root: %s\n", kind_label(child->kind));
        exit(1);
    }
  }
}

void
analyze(Node *ast)
{
  analyzer = (Analyzer){
    .scope = create_dict(),
  };
  analyze_root(ast);
}
