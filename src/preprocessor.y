%{
// preprocessor.y is a parser that provides `pp_parse_exp` function,
// which is used by lexer.l to parse `#if`'s parameter.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clannad.h"
int pplex(void);
int pperror(char const *str);
static Node *ppresult;

void set_type(Vector *nodes, Node *type);
Node* create_decl_node(Node *spec, Node *init);

%}

%union {
  Node *node;
  char *id;
  long ival;
  double fval;
  Vector *list;
  char ch;
}

%token <id>   tINT
%token <id>   tCHAR
%token <id>   tVOID
%token <id>   tIF
%token <id>   tELSE
%token <id>   tRETURN
%token <ival> tI_CONSTANT
%token <fval> tF_CONSTANT
%token <id>   tIDENTIFIER
%token <id>   tSTRING_LITERAL
%token <id>   tDEFINED
%token <id>   tSIZEOF
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
%type <node> integer_constant
%type <id>   type_specifier

%start program

%%

program
  : expression
  {
    ppresult = $1;
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
  | tDEFINED '(' tIDENTIFIER ')'
  {
    $$ = create_node(&(Node){ NODE_DEFINED, .id = $3 });
  }
  | tDEFINED tIDENTIFIER
  {
    $$ = create_node(&(Node){ NODE_DEFINED, .id = $2 });
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
  : integer_constant
  ;

string
  : tSTRING_LITERAL
  {
    $$ = create_node(&(Node){ NODE_STRING, .id = $1 });
  }
  ;

integer_constant
  : tI_CONSTANT
  {
    $$ = create_node(&(Node){ NODE_INTEGER, .ival = $1 });
  }
  | tF_CONSTANT
  {
    $$ = create_node(&(Node){ NODE_INTEGER, .fval = $1 });
  }
  ;

type_specifier
  : tINT
  {
    $$ = "int";
  }
  | tCHAR
  {
    $$ = "char";
  }
  | tVOID
  {
    $$ = "void";
  }
  ;

%%

int
pperror(char const *str)
{
  extern char *pptext;
  fprintf(stderr, "Preprocessor parse error near '%s': %s\n", pptext, str);
  return 0;
}

int
pp_parse_exp(Node **astptr, char *exp)
{
  extern int ppparse(void);
  extern FILE *ppin;

  ppin = fmemopen(exp, strlen(exp), "r");
  if (!ppin) {
    fprintf(stderr, "Failed to fmemopen: '%s'\n", exp);
    return 1;
  }
  int ret = ppparse();
  *astptr = ppresult;

  fclose(ppin);
  return ret;
}
