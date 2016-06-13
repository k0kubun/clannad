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
}

%token <id> tINT
%token <id> tIDENTIFIER

%type <id> type_specifier
%type <node> translation_unit
%type <node> declaration_specifiers external_declaration function_definition
%type <node> declarator direct_declarator compound_statement

%start translation_unit

%%

translation_unit
  : external_declaration
  {
    vector_push(parse_result->children, $1);
  }
  ;

external_declaration
  : function_definition
  ;

function_definition
  : declaration_specifiers declarator compound_statement
  {
    Node *node  = create_node(&(Node){ NODE_FUNC });
    node->spec  = $1;
    node->decl  = $2;
    node->stmts = $3;
    $$ = node;
  }
  ;

declaration_specifiers
  : type_specifier
  {
    Node *node = create_node(&(Node){ NODE_TYPE });
    node->id = $1;
    $$ = node;
  }
  ;

declarator
  : direct_declarator
  ;

direct_declarator
  : tIDENTIFIER
  {
    Node *node = create_node(&(Node){ NODE_DECL });
    node->id = yyval.id;
    $$ = node;
  }
  | direct_declarator '(' ')'
  ;

compound_statement
  : '{' '}'
  {
    $$ = create_node(&(Node){ NODE_COMPOUND_STMT });
  }
  ;

type_specifier
  : tINT
  {
    $$ = "int";
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
  fprintf(stderr, "parser error near %s\n", yytext);
  return 0;
}

int
parse_stdin(Node **astptr)
{
  extern int yyparse(void);
  extern FILE *yyin;

  yyin = stdin;
  parse_result = create_node(&(Node){ NODE_ROOT, create_vector() });
  int ret = yyparse();
  *astptr = parse_result;
  return ret;
}
