#ifndef CLANNAD_H
#define CLANNAD_H

enum NodeType {
  NODE_ROOT,
  NODE_FUNC,
  NODE_TYPE,
  NODE_DECL,
  NODE_COMPOUND_STMT,
};

typedef struct {
  enum NodeType type;
} Node;

// debug.c
void dump_ast(Node *ast);

// parser.y
int parse_stdin(Node **astptr);

#endif
