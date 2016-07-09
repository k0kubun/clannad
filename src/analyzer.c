#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clannad.h"

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
analyze_unary(Node *node)
{
  assert_node(node, NODE_UNARY);
}

void
analyze_binop(Node *node)
{
  assert_node(node, NODE_BINOP);
}

void
analyze_int(Node *node)
{
  assert_node(node, NODE_INTEGER);
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
}

void
analyze_funcall(Node *node)
{
  assert_node(node, NODE_FUNCALL);
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
    case NODE_STRING:
      return analyze_string(node);
    case NODE_IDENTIFIER:
      return analyze_variable(node);
    case NODE_FUNCALL:
      return analyze_funcall(node);
    default:
      fprintf(stderr, "Unexpected node in analyze_exp: %s\n", kind_label(node->kind));
      exit(1);
  }
}

void
analyze_comp_stmt(Node *node)
{
  assert_node(node, NODE_COMPOUND_STMT);
}

void
analyze_if(Node *node)
{
  assert_node(node, NODE_IF);
}

void
analyze_return(Node *node)
{
  assert_node(node, NODE_RETURN);
}

void
analyze_var_decl(Node *node)
{
  assert_node(node, NODE_VAR_DECL);
}

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
    case NODE_VAR_DECL:
      analyze_var_decl(node);
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
    if (!strcmp(first->type->id, "void") && !first->spec) {
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
  analyze_stmt(node->stmts);

  if (!strcmp(node->type->id, "void")) {
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
analyze_root(Node *node)
{
  assert_node(node, NODE_ROOT);

  for (int i = 0; i < node->children->length; i++) {
    Node *child = (Node *)vector_get(node->children, i);
    switch (child->kind) {
      case NODE_FUNC:
        analyze_func(child);
        break;
      case NODE_FUNC_DECL:
        analyze_func_decl(child);
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
  analyze_root(ast);
}
