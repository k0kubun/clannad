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
  NODE_TYPEDEF,
  NODE_STRUCT,
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
#define TYPE_VOID       (1L<<1)
#define TYPE_CHAR       (1L<<2)
#define TYPE_SHORT      (1L<<3)
#define TYPE_INT        (1L<<4)
#define TYPE_LONG       (1L<<6)
#define TYPE_SIGNED     (1L<<7)
#define TYPE_UNSIGNED   (1L<<8)
#define TYPE_FLOAT      (1L<<9)
#define TYPE_DOUBLE     (1L<<10)
#define TYPE_TYPEDEF    (1L<<11)
#define TYPE_STRUCT     (1L<<12)
#define TYPE_UNION      (1L<<13)
#define TYPE_CONST      (1L<<14)
#define TYPE_VOLATILE   (1L<<15)

typedef struct {
  void **data;
  int length;
} Vector;

typedef struct Dict {
  Vector *entries;
  struct Dict *parent;
} Dict;

typedef struct {
  char *key;
  void *data;
} DictEntry;

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
    // NODE_SPEC, NODE_TYPE, NODE_IDENTIFIER, NODE_STRING, NODE_DEFINED, NODE_STRUCT
    struct {
      char *id;
      union {
        struct Node *ref_node; // NODE_IDENTIFIER only
        long flags; // NODE_TYPE only
        struct { // NODE_STRUCT
          Vector *fields;
          bool is_union;
        };
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

// compiler.c
LLVMModuleRef compile(Node *ast);

// debug.c
void dump_ast(Node *ast);
char* kind_label(enum NodeKind kind);
void dump_macros();

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
extern int yyget_lineno(void);
Macro* find_macro(char *key);
Dict* build_macro_subst(Macro *macro, Vector *args);
void push_macros(Dict *new_macros);
void set_macros(Dict *new_macros);
Dict* check_macros();

// optimizer.c
void optimize(LLVMModuleRef mod);

// parser.y
Node* create_node(Node *temp);
int parse_file(Node **astptr, char *filename);
bool has_typedef(char *name);

// preprocessor.y
int pp_parse_exp(Node **astptr, char *exp);

// vector.c
Vector* create_vector();
Vector* vector_push(Vector *vec, void *ptr);
void* vector_get(Vector *vec, int index);
void* vector_last(Vector *vec);

#endif
