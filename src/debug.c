#include <stdio.h>
#include "clannad.h"

void
dump_ast_with_indent(Node *ast, int indent)
{
  switch (ast->type) {
    case NODE_ROOT:
      printf("NODE_ROOT\n");
      break;
    case NODE_FUNC:
      printf("NODE_FUNC\n");
      break;
    case NODE_TYPE:
      printf("NODE_TYPE\n");
      break;
    case NODE_DECL:
      printf("NODE_DECL\n");
      break;
    case NODE_COMPOUND_STMT:
      printf("NODE_TYPE\n");
      break;
    default:
      printf("default\n");
      break;
  }
}

void
dump_ast(Node *ast)
{
  dump_ast_with_indent(ast, 0);
}
