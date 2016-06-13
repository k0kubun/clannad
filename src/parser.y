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
%type <node> program translation_unit
%type <node> declaration_specifiers external_declaration function_definition
%type <node> declarator direct_declarator compound_statement

%start program

%%

program
  : translation_unit
  {
    parse_result = $1;
  }

translation_unit
  : external_declaration
  ;

external_declaration
  : function_definition
  ;

function_definition
  : declaration_specifiers declarator compound_statement
  {
    $$ = create_node(&(Node){ NODE_DECL });
  }
  ;

declaration_specifiers
  : type_specifier
  {
    $$ = create_node(&(Node){ NODE_TYPE });
  }
  ;

declarator
  : direct_declarator
  ;

direct_declarator
  : tIDENTIFIER
  {
    $$ = create_node(&(Node){ NODE_DECL });
  }
  | direct_declarator '(' ')'
  {
    $$ = create_node(&(Node){ NODE_DECL });
  }
  ;

compound_statement
  : '{' '}'
  {
    $$ = create_node(&(Node){ NODE_COMPOUND_STMT });
  }
  ;

type_specifier
  : tINT
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
  int ret;
  extern int yyparse(void);
  extern FILE *yyin;

  yyin = stdin;
  ret = yyparse();
  *astptr = parse_result;
  return ret;
}
