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
  void **data;
  int length;
} Vector;

typedef struct Node {
  enum NodeType type;
  union {
    // NODE_ROOT
    Vector *children;
    // NODE_FUNC
    struct {
      struct Node *spec;
      struct Node *decl;
      struct Node *stmts;
    };
    // NODE_DECL, NODE_TYPE
    char *id;
  };
} Node;

// debug.c
void dump_ast(Node *ast);

// parser.y
int parse_stdin(Node **astptr);

// vector.h
Vector* create_vector();
void* vector_get(Vector *vec, int index);
void vector_push(Vector *vec, void *ptr);

#endif
