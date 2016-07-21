#include <stdarg.h>
#include <stdlib.h>
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

void
indented_putc(int indent, const char c)
{
  indented_printf(indent, "%c\n", c);
}

char*
kind_label(enum NodeKind type)
{
  switch (type) {
    case NODE_ROOT:
      return "NODE_ROOT";
    case NODE_DECLN:
      return "NODE_DECLN";
    case NODE_FUNC:
      return "NODE_FUNC";
    case NODE_FUNC_DECL:
      return "NODE_FUNC_DECL";
    case NODE_PARAM_DECL:
      return "NODE_PARAM_DECL";
    case NODE_VAR_DECL:
      return "NODE_VAR_DECL";
    case NODE_FUNCALL:
      return "NODE_FUNCALL";
    case NODE_TYPE:
      return "NODE_TYPE";
    case NODE_PTR:
      return "NODE_PTR";
    case NODE_SPEC:
      return "NODE_SPEC";
    case NODE_FUNC_SPEC:
      return "NODE_FUNC_SPEC";
    case NODE_COMPOUND_STMT:
      return "NODE_COMPOUND_STMT";
    case NODE_RETURN:
      return "NODE_RETURN";
    case NODE_IDENTIFIER:
      return "NODE_IDENTIFIER";
    case NODE_INTEGER:
      return "NODE_INTEGER";
    case NODE_FLOAT:
      return "NODE_FLOAT";
    case NODE_STRING:
      return "NODE_STRING";
    case NODE_BINOP:
      return "NODE_BINOP";
    case NODE_UNARY:
      return "NODE_UNARY";
    case NODE_IF:
      return "NODE_IF";
    case NODE_TERNARY:
      return "NODE_TERNARY";
    case NODE_COMMA:
      return "NODE_COMMA";
    case NODE_TYPEDEF:
      return "NODE_TYPEDEF";
    case NODE_FIELD:
      return "NODE_FIELD";
    case NODE_FIELD_REF:
      return "NODE_FIELD_REF";
    case NODE_DEFINED:
      return "NODE_DEFINED";
    case NODE_ARRAY_SPEC:
      return "NODE_ARRAY_SPEC";
    case NODE_ARRAY_REF:
      return "NODE_ARRAY_REF";
    default:
      fprintf(stderr, "kind_label is not defined for %d\n", type);
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
print_types(Node *node)
{
  assert_node(node, NODE_TYPE);

  printf(" (");
  if (node->flags & TYPE_VOID    ) printf(" TYPE_VOID");
  if (node->flags & TYPE_CHAR    ) printf(" TYPE_CHAR");
  if (node->flags & TYPE_SHORT   ) printf(" TYPE_SHORT");
  if (node->flags & TYPE_INT     ) printf(" TYPE_INT");
  if (node->flags & TYPE_LONG    ) printf(" TYPE_LONG");
  if (node->flags & TYPE_SIGNED  ) printf(" TYPE_SIGNED");
  if (node->flags & TYPE_UNSIGNED) printf(" TYPE_UNSIGNED");
  if (node->flags & TYPE_FLOAT   ) printf(" TYPE_FLOAT");
  if (node->flags & TYPE_DOUBLE  ) printf(" TYPE_DOUBLE");
  if (node->flags & TYPE_TYPEDEF ) printf(" TYPE_TYPEDEF");
  if (node->flags & TYPE_STRUCT  ) printf(" TYPE_STRUCT");
  if (node->flags & TYPE_UNION   ) printf(" TYPE_UNION");
  if (node->flags & TYPE_CONST   ) printf(" TYPE_CONST");
  if (node->flags & TYPE_VOLATILE) printf(" TYPE_VOLATILE");
  printf(" )");
}

void
dump_node(int indent, Node *node)
{
  switch (node->kind) {
    case NODE_ROOT:
    case NODE_DECLN:
    case NODE_COMPOUND_STMT:
      indented_puts(indent, kind_label(node->kind));
      dump_vector(indent + 1, node->children);
      break;
    case NODE_PTR:
    case NODE_RETURN:
      indented_puts(indent, kind_label(node->kind));
      if (node->param) dump_node(indent + 1, node->param);
      break;
    case NODE_FUNC:
      indented_puts(indent, kind_label(node->kind));
      dump_nodes(indent + 1, 3, node->type, node->spec, node->stmts);
      break;
    case NODE_PARAM_DECL:
      indented_puts(indent, kind_label(node->kind));
      dump_node(indent + 1, node->type);
      if (node->spec) dump_node(indent + 1, node->spec);
      break;
    case NODE_FUNC_DECL:
    case NODE_VAR_DECL:
      indented_puts(indent, kind_label(node->kind));
      dump_nodes(indent + 1, 2, node->type, node->spec);
      break;
    case NODE_FUNCALL:
    case NODE_FUNC_SPEC:
      indented_puts(indent, kind_label(node->kind));
      dump_node(indent + 1, node->func);
      dump_vector(indent + 1, node->params);
      break;
    case NODE_TYPE:
      indented_printf(indent, "%s", kind_label(node->kind));
      print_types(node);
      printf("\n");
      if (node->fields) dump_vector(indent + 1, node->fields);
      break;
    case NODE_FIELD:
      indented_printf(indent, "%s\n", kind_label(node->kind));
      dump_node(indent + 1, node->field_type);
      dump_vector(indent + 1, node->fields);
      break;
    case NODE_IDENTIFIER:
    case NODE_SPEC:
    case NODE_STRING:
    case NODE_DEFINED:
      indented_printf(indent, "%s id=%s\n", kind_label(node->kind), node->id);
      break;
    case NODE_INTEGER:
      indented_printf(indent, "%s ival=%d\n", kind_label(node->kind), node->ival);
      break;
    case NODE_BINOP:
      indented_printf(indent, "%s '%c'\n", kind_label(node->kind), node->op);
      dump_node(indent + 1, node->lhs);
      dump_node(indent + 1, node->rhs);
      break;
    case NODE_UNARY:
      if ((int)node->op >= 256) {
        indented_printf(indent, "%s token %d\n", kind_label(node->kind), (int)node->op);
      } else {
        indented_printf(indent, "%s %c\n", kind_label(node->kind), node->op);
      }
      if (node->lhs) dump_node(indent + 1, node->lhs);
      if (node->rhs) dump_node(indent + 1, node->rhs);
      break;
    case NODE_IF:
      indented_puts(indent, kind_label(node->kind));
      dump_node(indent + 1, node->cond);
      dump_node(indent + 1, node->if_stmt);
      if (node->else_stmt) dump_node(indent + 1, node->else_stmt);
      break;
    case NODE_ARRAY_SPEC:
      indented_puts(indent, kind_label(node->kind));
      dump_node(indent + 1, node->lhs);
      dump_node(indent + 1, node->rhs);
      break;
    default:
      indented_puts(indent, kind_label(node->kind));
      break;
  }
}

void
dump_ast(Node *ast)
{
  dump_node(0, ast);
}

void
dump_macros()
{
  int depth = 0;
  for (Dict *macros = check_macros(); macros; macros = macros->parent, depth++) {
    for (int i = 0; i < macros->entries->length; i++) {
      DictEntry *entry = vector_get(macros->entries, i);
      Macro *macro = entry->data;
      for (int j = 0; j < depth; j++) printf(" ");
      printf("'%s' => '%s'\n", entry->key, macro->value);
    }
  }
}

void
dump_typedefs()
{
  Dict *defs = init_typedefs();
  for (int i = 0; i < defs->entries->length; i++) {
    DictEntry *entry = vector_get(defs->entries, i);
    printf("'%s':\n", entry->key);
    Node *type = entry->data;
    dump_ast(type);
  }
}
