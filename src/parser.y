%{
#include <stdio.h>
#include <stdlib.h>
#include "clannad.h"
int yylex(void);
int yyerror(char const *str);
static Node* create_node(Node *temp);
static Node *parse_result;
%}

%union {
  Node *node;
  char *id;
  long ival;
  Vector *list;
}

%token <id>   tINT
%token <id>   tCHAR
%token <id>   tIF
%token <id>   tELSE
%token <id>   tRETURN
%token <ival> tINTEGER
%token <id>   tIDENTIFIER
%token <id>   tSTRING_LITERAL
%token <id>   tINC_OP
%token <id>   tDEC_OP
%token <id>   tEQ_OP
%token <id>   tNE_OP

%type <list> translation_unit
%type <node> declaration_specifiers
%type <node> external_declaration
%type <node> function_definition
%type <node> declaration
%type <node> init_declarator_list
%type <node> init_declarator
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
%type <node> assignment_expression
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
  ;

relational_expression
  : shift_expression
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
  ;

cast_expression
  : unary_expression
  ;

unary_expression
  : postfix_expression
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
    $$ = create_node(&(Node){ NODE_UNARY, .lhs = $1, .op = INC_OP, .rhs = NULL });
  }
  | postfix_expression tDEC_OP
  {
    $$ = create_node(&(Node){ NODE_UNARY, .lhs = $1, .op = DEC_OP, .rhs = NULL });
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

/* FIXME: skipping many reductions before equality_expression */
assignment_expression
  : equality_expression
  | unary_expression '=' assignment_expression
  {
    $$ = create_node(&(Node){ NODE_BINOP, .lhs = $1, .op = '=', .rhs = $3 });
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
  ;

%%

static Node*
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
