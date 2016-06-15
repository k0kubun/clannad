#ifndef CLANNAD_H
#define CLANNAD_H

#include <llvm-c/Core.h>

enum NodeType {
  NODE_ROOT,
  NODE_FUNC,
  NODE_FUNCALL,
  NODE_TYPE,
  NODE_DECL,
  NODE_RETURN,
  NODE_INTEGER,
  NODE_STRING,
  NODE_IDENTIFIER,
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
    // NODE_FUNCALL
    struct {
      Vector *params;
      struct Node *func;
    };
    // NODE_RETURN
    struct Node *param;
    // NODE_FUNC
    struct {
      struct Node *spec;
      struct Node *decl;
      struct Node *stmts;
    };
    // NODE_DECL, NODE_TYPE, NODE_IDENTIFIER, NODE_STRING
    char *id;
    // NODE_INTEGER
    long ival;
  };
} Node;

// debug.c
void dump_ast(Node *ast);
char* type_label(enum NodeType type);

// parser.y
int parse_stdin(Node **astptr);

// compiler.c
LLVMModuleRef compile(Node *ast);

// vector.c
Vector* create_vector();
Vector* vector_push(Vector *vec, void *ptr);
void* vector_get(Vector *vec, int index);

#endif
