#ifndef CLANNAD_H
#define CLANNAD_H

enum NodeType {
  NODE_ROOT,
  NODE_FUNC,
  NODE_TYPE,
  NODE_DECL,
  NODE_RETURN,
  NODE_INTEGER,
  NODE_COMPOUND_STMT,
};

typedef struct {
  void **data;
  int length;
} Vector;

typedef struct Node {
  enum NodeType type;
  union {
    // NODE_ROOT, NODE_COUMPOUND_STMT
    Vector *children;
    // NODE_RETURN
    struct Node *param;
    // NODE_FUNC
    struct {
      struct Node *spec;
      struct Node *decl;
      struct Node *stmts;
    };
    // NODE_DECL, NODE_TYPE
    char *id;
    // NODE_INTEGER
    long ival;
  };
} Node;

// debug.c
void dump_ast(Node *ast);

// parser.y
int parse_stdin(Node **astptr);

// vector.h
Vector* create_vector();
Vector* vector_push(Vector *vec, void *ptr);
void* vector_get(Vector *vec, int index);

#endif
