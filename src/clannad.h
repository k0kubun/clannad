#ifndef CLANNAD_H
#define CLANNAD_H

#include <stdio.h>
#include <llvm-c/Core.h>

enum NodeKind {
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
  NODE_IF,
  NODE_UNARY,
};

enum DoubleCharOp {
  SIZEOF = 256,
  INC_OP,
  DEC_OP,
  EQ_OP,
  NE_OP,
  LE_OP,
  GE_OP,
  AND_OP,
  OR_OP,
  LEFT_OP,
  RIGHT_OP,
};

typedef struct {
  void **data;
  int length;
} Vector;

typedef struct {
  Vector *entries;
} Dict;

typedef struct Node {
  enum NodeKind kind;
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
      struct Node *type;
      struct Node *spec;
      struct Node *stmts; // NODE_FUNC only
    };
    // NODE_SPEC, NODE_TYPE, NODE_IDENTIFIER, NODE_STRING
    char *id;
    // NODE_INTEGER
    long ival;
    // NODE_BINOP, NODE_UNARY
    struct {
      struct Node *lhs;
      int op;
      struct Node *rhs;
    };
    // NODE_IF
    struct {
      struct Node *cond;
      struct Node *if_stmt;
      struct Node *else_stmt;
    };
  };
} Node;

// analyzer.c
void analyze(Node *ast);
void assert_node(Node *node, enum NodeKind kind);

// assembler.c
void assemble(LLVMModuleRef mod, char *outfile);

// debug.c
void dump_ast(Node *ast);
char* kind_label(enum NodeKind kind);

// parser.y
int parse_file(Node **astptr, FILE *file);

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

// optimizer.c
void optimize(LLVMModuleRef mod);

#endif
