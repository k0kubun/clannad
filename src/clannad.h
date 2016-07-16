#ifndef CLANNAD_H
#define CLANNAD_H

#include <stdio.h>
#include <stdbool.h>
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
  NODE_FLOAT,
  NODE_STRING,
  NODE_IDENTIFIER,
  NODE_COMPOUND_STMT,
  NODE_BINOP,
  NODE_IF,
  NODE_UNARY,
  NODE_DECLN,
  NODE_TERNARY,
  NODE_COMMA,
  NODE_DEFINED, // preprocessor only
};

enum MultiCharsOp {
  SIZEOF = 256,
  EQ_OP,
  NE_OP,
  LE_OP,
  GE_OP,
  AND_OP,
  OR_OP,
  LEFT_OP,
  RIGHT_OP,
  PRE_INC_OP,
  PRE_DEC_OP,
  POST_INC_OP,
  POST_DEC_OP,
  RIGHT_ASSIGN,
  LEFT_ASSIGN,
  ADD_ASSIGN,
  SUB_ASSIGN,
  MUL_ASSIGN,
  DIV_ASSIGN,
  MOD_ASSIGN,
  AND_ASSIGN,
  XOR_ASSIGN,
  OR_ASSIGN,
};

// NODE_TYPE's flags
static const int TYPE_CONST    = 1;
static const int TYPE_VOLATILE = 2;

typedef struct {
  void **data;
  int length;
} Vector;

typedef struct Dict {
  Vector *entries;
  struct Dict *parent;
} Dict;

typedef struct Macro {
  char *value;
  bool is_function;
  Vector *params;
} Macro;

typedef struct Node {
  enum NodeKind kind;
  LLVMValueRef ref;
  union {
    // NODE_ROOT, NODE_COUMPOUND_STMT, NODE_DECLN
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
      union {
        struct Node *init; // NODE_VAR_DECL only
        struct Node *stmts; // NODE_FUNC only
      };
    };
    // NODE_SPEC, NODE_TYPE, NODE_IDENTIFIER, NODE_STRING, NODE_DEFINED
    struct {
      char *id;
      union {
        struct Node *ref_node; // NODE_IDENTIFIER only
        int flags; // NODE_TYPE only
      };
    };
    // NODE_INTEGER
    long ival;
    // NODE_FLOAT
    double fval;
    // NODE_BINOP, NODE_UNARY, NODE_COMMA
    struct {
      int op;
      union {
        struct Node *val; // for NODE_UNARY
        struct { // for NODE_BINOP, NODE_COMMA
          struct Node *lhs;
          struct Node *rhs;
        };
      };
    };
    // NODE_IF, NODE_TERNARY
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
Node* create_node(Node *temp);
int parse_file(Node **astptr, char *filename);

// compiler.c
LLVMModuleRef compile(Node *ast);

// dict.c
Dict* create_dict();
void* dict_get(Dict *dict, char *key);
void dict_set(Dict *dict, char *key, void *value);
bool dict_delete(Dict *dict, char *key);

// file.c
FILE* open_file(char *filename);
FILE* drop_backslash_newline(FILE *fp);

// lexer.l
void init_search_paths();
void set_compile_path(char *filename);
char* get_reading_file();
Macro* find_macro(char *key);
Dict* build_macro_subst(Macro *macro, Vector *args);
void push_macros(Dict *new_macros);

// optimizer.c
void optimize(LLVMModuleRef mod);

// preprocessor.y
int pp_parse_exp(Node **astptr, char *exp);

// vector.c
Vector* create_vector();
Vector* vector_push(Vector *vec, void *ptr);
void* vector_get(Vector *vec, int index);
void* vector_last(Vector *vec);

#endif
