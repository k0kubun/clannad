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
    case NODE_TYPE:
      return "NODE_TYPE";
    case NODE_DECL:
      return "NODE_DECL";
    case NODE_COMPOUND_STMT:
      return "NODE_COMPOUND_STMT";
    default:
      fprintf(stderr, "node_name->type: (%d)", node->type);
      return "NODE_UNSUPPORTED";
  }
}

void
dump_node_with_indent(Node *node, int indent)
{
  switch (node->type) {
    case NODE_ROOT:
      indented_puts(indent, node_name(node));
      for (int i = 0; i < node->children->length; i++) {
        dump_node_with_indent((Node *)vector_get(node->children, i), indent + 1);
      }
      break;
    case NODE_FUNC:
      indented_puts(indent, node_name(node));
      dump_node_with_indent(node->spec, indent + 1);
      dump_node_with_indent(node->decl, indent + 1);
      dump_node_with_indent(node->stmts, indent + 1);
      break;
    case NODE_DECL:
    case NODE_TYPE:
      indented_printf(indent, "%s id=%s\n", node_name(node), node->id);
      break;
    default:
      indented_puts(indent, node_name(node));
      break;
  }
}

void
dump_ast(Node *ast)
{
  dump_node_with_indent(ast, 0);
}
