#include <stdarg.h>
#include <stdio.h>
#include "clannad.h"

void
indented_printf(int indent, const char *format, ...)
{
  for (int i = 0; i < indent; i++) printf("  ");
  va_list list;
  va_start(list, format);
  vprintf(format, list);
  va_end(list);
}

void
indented_puts(int indent, const char *str)
{
  indented_printf(indent, "%s\n", str);
}

char*
node_name(Node *node)
{
  switch (node->type) {
    case NODE_ROOT:
      return "NODE_ROOT";
    case NODE_FUNC:
      return "NODE_FUNC";
    case NODE_FUNCALL:
      return "NODE_FUNCALL";
    case NODE_TYPE:
      return "NODE_TYPE";
    case NODE_DECL:
      return "NODE_DECL";
    case NODE_COMPOUND_STMT:
      return "NODE_COMPOUND_STMT";
    case NODE_RETURN:
      return "NODE_RETURN";
    case NODE_IDENTIFIER:
      return "NODE_IDENTIFIER";
    case NODE_INTEGER:
      return "NODE_INTEGER";
    default:
      fprintf(stderr, "unexpected node type: %d\n", node->type);
      return "NODE_UNSUPPORTED";
  }
}

void dump_node(int indent, Node *node);

void
dump_vector(int indent, Vector *vec)
{
  for (int i = 0; i < vec->length; i++) {
    dump_node(indent, (Node *)vector_get(vec, i));
  }
}

void
dump_nodes(int indent, int argc, ...)
{
  va_list list;
  va_start(list, argc);
  for (int i = 0; i < argc; i++) dump_node(indent, va_arg(list, Node *));
  va_end(list);
}

void
dump_node(int indent, Node *node)
{
  switch (node->type) {
    case NODE_ROOT:
    case NODE_COMPOUND_STMT:
      indented_puts(indent, node_name(node));
      dump_vector(indent + 1, node->children);
      break;
    case NODE_RETURN:
      indented_puts(indent, node_name(node));
      if (node->param) dump_node(indent + 1, node->param);
      break;
    case NODE_FUNC:
      indented_puts(indent, node_name(node));
      dump_nodes(indent + 1, 3, node->spec, node->decl, node->stmts);
      break;
    case NODE_FUNCALL:
      indented_puts(indent, node_name(node));
      dump_node(indent + 1, node->func);
      dump_vector(indent + 1, node->params);
      break;
    case NODE_IDENTIFIER:
    case NODE_DECL:
    case NODE_TYPE:
      indented_printf(indent, "%s id=%s\n", node_name(node), node->id);
      break;
    case NODE_INTEGER:
      indented_printf(indent, "%s ival=%d\n", node_name(node), node->ival);
      break;
    default:
      indented_puts(indent, node_name(node));
      break;
  }
}

void
dump_ast(Node *ast)
{
  dump_node(0, ast);
}
