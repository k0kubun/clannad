%{
#include <stdio.h>
#include <stdlib.h>
#include "clannad.h"
int yylex(void);
int yyerror(char const *str);
static Node *parse_result;
%}

%union {
  Node *node;
  char *id;
  long ival;
  Vector *list;
  char ch;
}

%token <id>   tINT
%token <id>   tCHAR
%token <id>   tVOID
%token <id>   tIF
%token <id>   tELSE
%token <id>   tRETURN
%token <ival> tINTEGER
%token <id>   tIDENTIFIER
%token <id>   tSTRING_LITERAL
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

%type <list> translation_unit
%type <node> declaration_specifiers
%type <node> external_declaration
%type <node> function_definition
%type <node> declaration
%type <node> init_declarator_list
%type <node> init_declarator
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
%type <node> integer_constant
%type <id>   type_specifier

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
    switch ($2->kind) {
    case NODE_FUNC_SPEC:
      $$ = create_node(&(Node){ NODE_FUNC_DECL, .type = $1, .spec = $2 });
      break;
    case NODE_SPEC:
      $$ = create_node(&(Node){ NODE_VAR_DECL, .type = $1, .spec = $2 });
      break;
    default:
      yyerror("unexpected decl type in declaration");
    }
  }
  ;

init_declarator_list
  : init_declarator
  ;

init_declarator
  : declarator
  ;

declaration_specifiers
  : type_specifier
  {
    $$ = create_node(&(Node){ NODE_TYPE, .id = $1 });
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
  : tINTEGER
  {
    $$ = create_node(&(Node){ NODE_INTEGER, .ival = $1 });
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
  fprintf(stderr, "parser error near '%s': %s\n", yytext, str);
  return 0;
}

int
parse_file(Node **astptr, FILE *file)
{
  extern int yyparse(void);
  extern FILE *yyin;

  yyin = file;
  int ret = yyparse();
  *astptr = parse_result;
  return ret;
}
