%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clannad.h"
int yylex(void);
int yyerror(char const *str);
static Node *parse_result;
static Dict *typedefs = NULL;
void handle_typedef(Node *type, Vector *decls);

void
set_type(Vector *nodes, Node *type)
{
  for (int i = 0; i < nodes->length; i++) {
    Node *node = vector_get(nodes, i);
    node->type = type;
  }
}

Node*
create_decl_node(Node *spec, Node *init)
{
  // type must be set on "declaration"
  switch (spec->kind) {
    case NODE_FUNC_SPEC:
      return create_node(&(Node){ NODE_FUNC_DECL, .type = NULL, .spec = spec, .init = init });
    case NODE_SPEC:
      return create_node(&(Node){ NODE_VAR_DECL, .type = NULL, .spec = spec, .init = init });
    default:
      yyerror("unexpected decl type in init_declarator");
      exit(1);
  }
}

%}

%union {
  Node *node;
  char *id;
  long ival;
  double fval;
  Vector *list;
  char ch;
}

%token <id>   tCHAR
%token <id>   tSHORT
%token <id>   tINT
%token <id>   tLONG
%token <id>   tSIGNED
%token <id>   tUNSIGNED
%token <id>   tFLOAT
%token <id>   tDOUBLE
%token <id>   tVOID
%token <id>   tSTRUCT
%token <id>   tUNION
%token <id>   tIF
%token <id>   tELSE
%token <id>   tRETURN
%token <ival> tI_CONSTANT
%token <fval> tF_CONSTANT
%token <id>   tIDENTIFIER
%token <id>   tSTRING_LITERAL
%token <id>   tSIZEOF
%token <id>   tCONST
%token <id>   tVOLATILE
%token <id>   tINC_OP
%token <id>   tDEC_OP
%token <id>   tEQ_OP
%token <id>   tNE_OP
%token <id>   tLE_OP
%token <id>   tGE_OP
%token <id>   tAND_OP
%token <id>   tOR_OP
%token <id>   tLEFT_OP
%token <id>   tRIGHT_OP
%token <id>   tRIGHT_ASSIGN;
%token <id>   tLEFT_ASSIGN;
%token <id>   tADD_ASSIGN;
%token <id>   tSUB_ASSIGN;
%token <id>   tMUL_ASSIGN;
%token <id>   tDIV_ASSIGN;
%token <id>   tMOD_ASSIGN;
%token <id>   tAND_ASSIGN;
%token <id>   tXOR_ASSIGN;
%token <id>   tOR_ASSIGN;
%token <id>   tNEWLINE;
%token <id>   tTYPEDEF;
%token <id>   tTYPEDEF_NAME;

%type <list> translation_unit
%type <node> declaration_specifiers
%type <ival> type_qualifier
%type <ival> storage_class_specifier
%type <node> external_declaration
%type <node> function_definition
%type <node> declaration
%type <list> init_declarator_list
%type <node> init_declarator
%type <node> initializer
%type <node> abstract_declarator
%type <node> declarator
%type <node> pointer
%type <node> direct_declarator
%type <list> parameter_type_list
%type <list> parameter_list
%type <node> parameter_declaration
%type <node> compound_statement
%type <list> block_item_list
%type <node> block_item
%type <node> statement
%type <node> expression_statement
%type <node> selection_statement
%type <node> jump_statement
%type <node> expression
%type <node> multiplicative_expression
%type <node> additive_expression
%type <node> shift_expression
%type <node> relational_expression
%type <node> equality_expression
%type <node> cast_expression
%type <node> unary_expression
%type <ch>   unary_operator
%type <node> type_name
%type <node> specifier_qualifier_list
%type <node> conditional_expression
%type <node> assignment_expression
%type <ival> assignment_operator
%type <node> logical_or_expression
%type <node> logical_and_expression
%type <node> and_expression
%type <node> inclusive_or_expression
%type <node> exclusive_or_expression
%type <node> postfix_expression
%type <list> argument_expression_list
%type <node> primary_expression
%type <node> constant
%type <node> string
%type <id>   type_specifier
%type <node> struct_or_union_specifier
%type <id>   struct_or_union
%type <list> struct_declaration_list
%type <node> struct_declaration
%type <list> struct_declarator_list
%type <node> struct_declarator

%start program

%%

program
  : translation_unit
  {
    parse_result = create_node(&(Node){ NODE_ROOT, .children = $1 });
  }
  ;

translation_unit
  : external_declaration
  {
    $$ = vector_push(create_vector(), $1);
  }
  | translation_unit external_declaration
  {
    $$ = vector_push($1, $2);
  }
  ;

external_declaration
  : function_definition
  | declaration
  ;

function_definition
  : declaration_specifiers declarator compound_statement
  {
    $$ = create_node(&(Node){ NODE_FUNC, .type = $1, .spec = $2, .stmts = $3 });
  }
  ;

declaration
  : declaration_specifiers init_declarator_list ';'
  {
    if ($1->flags & TYPE_TYPEDEF) {
      handle_typedef($1, $2);
      Node *node = create_node(&(Node){ NODE_TYPEDEF });
      $$ = create_node(&(Node){ NODE_DECLN, .children = vector_push(create_vector(), node) });
    } else {
      set_type($2, $1);
      $$ = create_node(&(Node){ NODE_DECLN, .children = $2 });
    }
  }
  ;

init_declarator_list
  : init_declarator
  {
    $$ = vector_push(create_vector(), $1);
  }
  | init_declarator_list ',' init_declarator
  {
    $$ = vector_push($1, $3);
  }
  ;

init_declarator
  : declarator '=' initializer
  {
    $$ = create_decl_node($1, $3);
  }
  | declarator
  {
    $$ = create_decl_node($1, NULL);
  }
  ;

initializer
  : assignment_expression
  ;

declaration_specifiers
  : storage_class_specifier declaration_specifiers
  {
    switch ($1) {
      case tTYPEDEF:
        $2->flags |= TYPE_TYPEDEF;
        break;
      default:
        yyerror("unexpected storage_class_specifier");
        exit(1);
    }
    $$ = $2;
  }
  | type_specifier declaration_specifiers
  {
    // FIXME: support multiple types properly
    if (has_typedef($1)) {
      $$ = dict_get(typedefs, $1);
    } else {
      $$ = create_node(&(Node){ NODE_TYPE, .id = $1, .flags = 0 });
    }
  }
  | type_specifier
  {
    if (has_typedef($1)) {
      $$ = dict_get(typedefs, $1);
    } else {
      $$ = create_node(&(Node){ NODE_TYPE, .id = $1, .flags = 0 });
    }
  }
  | type_qualifier declaration_specifiers
  {
    switch ($1) {
      case tCONST:
        $2->flags |= TYPE_CONST;
        break;
      case tVOLATILE:
        $2->flags |= TYPE_VOLATILE;
        break;
      default:
        yyerror("unexpected type_qualifier");
        exit(1);
    }
    $$ = $2;
  }
  ;

storage_class_specifier
  : tTYPEDEF
  {
    $$ = tTYPEDEF;
  }
  ;

type_qualifier
  : tCONST
  {
    $$ = tCONST;
  }
  | tVOLATILE
  {
    $$ = tVOLATILE;
  }
  ;

declarator
  : pointer direct_declarator
  {
    $1->param = $2;
    $$ = $1;
  }
  | direct_declarator
  ;

pointer
  : '*'
  {
    $$ = create_node(&(Node){ NODE_PTR, .param = NULL });
  }
  ;

direct_declarator
  : tIDENTIFIER
  {
    $$ = create_node(&(Node){ NODE_SPEC, .id = yyval.id });
  }
  | direct_declarator '(' ')'
  {
    $$ = create_node(&(Node){ NODE_FUNC_SPEC, .func = $1, .params = create_vector() });
  }
  | direct_declarator '(' parameter_type_list ')'
  {
    $$ = create_node(&(Node){ NODE_FUNC_SPEC, .func = $1, .params = $3 });
  }
  ;

parameter_type_list
  : parameter_list
  ;

parameter_list
  : parameter_declaration
  {
    $$ = vector_push(create_vector(), $1);
  }
  | parameter_list ',' parameter_declaration
  {
    $$ = vector_push($1, $3);
  }
  ;

parameter_declaration
  : declaration_specifiers declarator
  {
    $$ = create_node(&(Node){ NODE_PARAM_DECL, .type = $1, .spec = $2 });
  }
  | declaration_specifiers abstract_declarator
  {
    $$ = create_node(&(Node){ NODE_PARAM_DECL, .type = $1, .spec = $2 });
  }
  | declaration_specifiers
  {
    $$ = create_node(&(Node){ NODE_PARAM_DECL, .type = $1, .spec = NULL });
  }
  ;

abstract_declarator
  : pointer
  ;

compound_statement
  : '{' '}'
  {
    $$ = create_node(&(Node){ NODE_COMPOUND_STMT, .children = create_vector() });
  }
  | '{' block_item_list '}'
  {
    $$ = create_node(&(Node){ NODE_COMPOUND_STMT, .children = $2 });
  }
  ;

block_item_list
  : block_item
  {
    $$ = vector_push(create_vector(), $1);
  }
  | block_item_list block_item
  {
    $$ = vector_push($1, $2);
  }
  ;

block_item
  : declaration
  | statement
  ;

statement
  : expression_statement
  | compound_statement
  | selection_statement
  | jump_statement
  ;

jump_statement
  : tRETURN ';'
  {
    $$ = create_node(&(Node){ NODE_RETURN, .param = NULL });
  }
  | tRETURN expression ';'
  {
    $$ = create_node(&(Node){ NODE_RETURN, .param = $2 });
  }
  ;

expression_statement
  : ';'
  {
    $$ = NULL;
  }
  | expression ';'
  {
    $$ = $1;
  }
  ;

selection_statement
  : tIF '(' expression ')' statement tELSE statement
  {
    $$ = create_node(&(Node){ NODE_IF, .cond = $3, .if_stmt = $5, .else_stmt = $7 });
  }
  | tIF '(' expression ')' statement
  {
    $$ = create_node(&(Node){ NODE_IF, .cond = $3, .if_stmt = $5, .else_stmt = NULL });
  }
  ;

expression
  : assignment_expression
  | expression ',' assignment_expression
  {
    $$ = create_node(&(Node){ NODE_COMMA, .lhs = $1, .rhs = $3 });
  }
  ;

additive_expression
  : multiplicative_expression
  | additive_expression '+' multiplicative_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = '+', .rhs = $3 });
  }
  | additive_expression '-' multiplicative_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = '-', .rhs = $3 });
  }
  ;

shift_expression
  : additive_expression
  | shift_expression tLEFT_OP additive_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = LEFT_OP, .rhs = $3 });
  }
  | shift_expression tRIGHT_OP additive_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = RIGHT_OP, .rhs = $3 });
  }
  ;

relational_expression
  : shift_expression
  | relational_expression '<' shift_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = '<', .rhs = $3 });
  }
  | relational_expression '>' shift_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = '>', .rhs = $3 });
  }
  | relational_expression tLE_OP shift_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = LE_OP, .rhs = $3 });
  }
  | relational_expression tGE_OP shift_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = GE_OP, .rhs = $3 });
  }
  ;

equality_expression
  : relational_expression
  | equality_expression tEQ_OP relational_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = EQ_OP, .rhs = $3 });
  }
  | equality_expression tNE_OP relational_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = NE_OP, .rhs = $3 });
  }
  ;

multiplicative_expression
  : cast_expression
  | multiplicative_expression '*' cast_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = '*', .rhs = $3 });
  }
  | multiplicative_expression '/' cast_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = '/', .rhs = $3 });
  }
  | multiplicative_expression '%' cast_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = '%', .rhs = $3 });
  }
  ;

cast_expression
  : unary_expression
  ;

unary_expression
  : postfix_expression
  | tINC_OP unary_expression
  {
    $$ = create_node(&(Node){ NODE_UNARY, .val = $2, .op = PRE_INC_OP });
  }
  | tDEC_OP unary_expression
  {
    $$ = create_node(&(Node){ NODE_UNARY, .val = $2, .op = PRE_DEC_OP });
  }
  | unary_operator cast_expression
  {
    $$ = create_node(&(Node){ NODE_UNARY, .val = $2, .op = $1 });
  }
  | tSIZEOF '(' type_name ')'
  {
    $$ = create_node(&(Node){ NODE_UNARY, .val = $3, .op = SIZEOF });
  }
  ;

type_name
  : specifier_qualifier_list
  ;

specifier_qualifier_list
  : type_specifier
  {
    $$ = create_node(&(Node){ NODE_TYPE, .id = $1 });
  }
  ;

unary_operator
  : '+'
  {
    $$ = '+';
  }
  | '-'
  {
    $$ = '-';
  }
  | '~'
  {
    $$ = '~';
  }
  | '!'
  {
    $$ = '!';
  }
  ;

postfix_expression
  : primary_expression
  | postfix_expression '(' ')'
  {
    $$ = create_node(&(Node){ NODE_FUNCALL, .func = $1, .params = create_vector() });
  }
  | postfix_expression '(' argument_expression_list ')'
  {
    $$ = create_node(&(Node){ NODE_FUNCALL, .func = $1, .params = $3 });
  }
  | postfix_expression tINC_OP
  {
    $$ = create_node(&(Node){ NODE_UNARY, .val = $1, .op = POST_INC_OP });
  }
  | postfix_expression tDEC_OP
  {
    $$ = create_node(&(Node){ NODE_UNARY, .val = $1, .op = POST_DEC_OP });
  }
  ;

argument_expression_list
  : assignment_expression
  {
    $$ = vector_push(create_vector(), $1);
  }
  | argument_expression_list ',' assignment_expression
  {
    $$ = vector_push($1, $3);
  }
  ;

conditional_expression
  : logical_or_expression
  | logical_or_expression '?' expression ':' conditional_expression
  {
    $$ = create_node(&(Node){ NODE_TERNARY, .cond = $1, .if_stmt = $3, .else_stmt = $5 });
  }
  ;

assignment_expression
  : conditional_expression
  | unary_expression assignment_operator assignment_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = $2, .rhs = $3 });
  }
  ;

assignment_operator
  : '='
  {
    $$ = '=';
  }
  | tRIGHT_ASSIGN
  {
    $$ = RIGHT_ASSIGN;
  }
  | tLEFT_ASSIGN
  {
    $$ = LEFT_ASSIGN;
  }
  | tADD_ASSIGN
  {
    $$ = ADD_ASSIGN;
  }
  | tSUB_ASSIGN
  {
    $$ = SUB_ASSIGN;
  }
  | tMUL_ASSIGN
  {
    $$ = MUL_ASSIGN;
  }
  | tDIV_ASSIGN
  {
    $$ = DIV_ASSIGN;
  }
  | tMOD_ASSIGN
  {
    $$ = MOD_ASSIGN;
  }
  | tAND_ASSIGN
  {
    $$ = AND_ASSIGN;
  }
  | tXOR_ASSIGN
  {
    $$ = XOR_ASSIGN;
  }
  | tOR_ASSIGN
  {
    $$ = OR_ASSIGN;
  }
  ;

logical_or_expression
  : logical_and_expression
  | logical_or_expression tOR_OP logical_and_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = OR_OP, .rhs = $3 });
  }
  ;

logical_and_expression
  : inclusive_or_expression
  | logical_and_expression tAND_OP inclusive_or_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = AND_OP, .rhs = $3 });
  }
  ;

and_expression
  : equality_expression
  | and_expression '&' equality_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = '&', .rhs = $3 });
  }
  ;

exclusive_or_expression
  : and_expression
  | exclusive_or_expression '^' and_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = '^', .rhs = $3 });
  }
  ;

inclusive_or_expression
  : exclusive_or_expression
  | inclusive_or_expression '|' exclusive_or_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = '|', .rhs = $3 });
  }
  ;

primary_expression
  : tIDENTIFIER
  {
    $$ = create_node(&(Node){ NODE_IDENTIFIER, .id = $1 });
  }
  | constant
  | string
  | '(' expression ')'
  {
    $$ = $2;
  }
  ;

constant
  : tI_CONSTANT
  {
    $$ = create_node(&(Node){ NODE_INTEGER, .ival = $1 });
  }
  | tF_CONSTANT
  {
    $$ = create_node(&(Node){ NODE_FLOAT, .fval = $1 });
  }
  ;

string
  : tSTRING_LITERAL
  {
    $$ = create_node(&(Node){ NODE_STRING, .id = $1 });
  }
  ;

type_specifier
  : tVOID
  {
    $$ = "void";
  }
  | tCHAR
  {
    $$ = "char";
  }
  | tSHORT
  {
    $$ = "short";
  }
  | tINT
  {
    $$ = "int";
  }
  | tLONG
  {
    $$ = "long";
  }
  | tFLOAT
  {
    $$ = "float";
  }
  | tDOUBLE
  {
    $$ = "double";
  }
  | tSIGNED
  {
    $$ = "signed";
  }
  | tUNSIGNED
  {
    $$ = "unsigned";
  }
  | struct_or_union_specifier
  {
    $$ = "struct";
  }
  | tTYPEDEF_NAME
  ;

struct_or_union_specifier
  : struct_or_union '{' struct_declaration_list '}'
  {
    $$ = create_node(&(Node){ NODE_STRUCT });
  }
  | struct_or_union tIDENTIFIER '{' struct_declaration_list '}'
  {
    $$ = create_node(&(Node){ NODE_STRUCT });
  }
  | struct_or_union tIDENTIFIER
  {
    $$ = create_node(&(Node){ NODE_STRUCT });
  }
  ;

struct_or_union
  : tSTRUCT
  | tUNION
  ;

struct_declaration_list
  : struct_declaration
  {
    $$ = vector_push(create_vector(), $1);
  }
  | struct_declaration_list struct_declaration
  {
    $$ = vector_push($1, $2);
  }
  ;

struct_declaration
  : specifier_qualifier_list ';'
  | specifier_qualifier_list struct_declarator_list ';'
  ;

struct_declarator_list
  : struct_declarator
  {
    $$ = vector_push(create_vector(), $1);
  }
  | struct_declarator_list ',' struct_declarator
  {
    $$ = vector_push($1, $3);
  }
  ;

struct_declarator
  : declaration
  ;

%%

Node*
create_node(Node *temp)
{
  Node *ret = malloc(sizeof(Node));
  *ret = *temp;
  return ret;
}

int
yyerror(char const *str)
{
  extern char *yytext;
  fprintf(stderr, "%s:%d: parse error near '%s': %s\n", get_reading_file(), yyget_lineno(), yytext, str);
  return 0;
}

int
parse_file(Node **astptr, char *filename)
{
  extern int yyparse(void);
  extern FILE *yyin;

  init_search_paths();
  set_compile_path(filename);

  FILE *fp = open_file(filename);
  yyin = drop_backslash_newline(fp);

  int ret = yyparse();
  *astptr = parse_result;

  fclose(yyin);
  return ret;
}

void
handle_typedef(Node *type, Vector *decls)
{
  if (!typedefs) typedefs = create_dict();
  type->flags ^= TYPE_TYPEDEF;

  // FIXME: typedef should have scope
  for (int i = 0; i < decls->length; i++) {
    Node *decl = vector_get(decls, i);
    switch (decl->kind) {
      case NODE_VAR_DECL:
        dict_set(typedefs, decl->spec->id, type);
        break;
      default:
        yyerror("unexpected node kind in typedef");
        exit(1);
    }
  }
}

bool
has_typedef(char *name)
{
  if (!typedefs) typedefs = create_dict();

  Node *node = dict_get(typedefs, name);
  return node != NULL;
}
