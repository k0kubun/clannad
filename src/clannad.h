#ifndef CLANNAD_H
#define CLANNAD_H

#include <llvm-c/Core.h>

enum NodeType {
  NODE_ROOT,
  NODE_FUNC,
  NODE_FUNC_DECL,
  NODE_PARAM_DECL,
  NODE_VAR_DECL,
  NODE_FUNCALL,
  NODE_TYPE,
  NODE_PTR,
  NODE_SPEC,
  NODE_FUNC_SPEC,
  NODE_RETURN,
  NODE_INTEGER,
  NODE_STRING,
  NODE_IDENTIFIER,
  NODE_COMPOUND_STMT,
  NODE_BINOP,
};

typedef struct {
  void **data;
  int length;
} Vector;

typedef struct {
  Vector *entries;
} Dict;

typedef struct Node {
  enum NodeType type;
  union {
    // NODE_ROOT, NODE_COUMPOUND_STMT
    Vector *children;
    // NODE_FUNCALL, NODE_FUNC_SPEC
    struct {
      Vector *params;
      struct Node *func;
    };
    // NODE_RETURN, NODE_PTR
    struct Node *param;
    // NODE_FUNC, NODE_FUNC_DECL, NODE_PARAM_DECL, NODE_VAR_DECL
    struct {
      struct Node *spec;
      struct Node *decl;
      struct Node *stmts; // NODE_FUNC only
    };
    // NODE_SPEC, NODE_TYPE, NODE_IDENTIFIER, NODE_STRING
    char *id;
    // NODE_INTEGER
    long ival;
    // NODE_BINOP
    struct {
      struct Node *lhs;
      char op;
      struct Node *rhs;
    };
  };
} Node;

// debug.c
void dump_ast(Node *ast);
char* type_label(enum NodeType type);

// parser.y
int parse_stdin(Node **astptr);

// compiler.c
LLVMModuleRef compile(Node *ast);

// dict.c
Dict* create_dict();
void* dict_get(Dict *dict, char *key);
void dict_set(Dict *dict, char *key, void *value);

// vector.c
Vector* create_vector();
Vector* vector_push(Vector *vec, void *ptr);
void* vector_get(Vector *vec, int index);

#endif
