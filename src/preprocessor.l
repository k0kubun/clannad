%{
// preprocessor.l is a lexer for `pp_parse_exp` function,
// which is used by lexer.l to parse `#if`'s parameter.

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include "clannad.h"
#include "preprocessor.tab.h"

// Linked list to keep YY_BUFFER_STATE during `yy_switch_to_buffer` calls.
struct pp_buffer_node {
  struct pp_buffer_node *parent;
  YY_BUFFER_STATE buf;
  FILE *file;
};
static struct pp_buffer_node *pp_buffer_stack = NULL;

char* read_string_literal(char *str);
static bool defined_mode = false;
bool pp_push_string(char *str);
bool pp_pop_string();
%}

%x COMMENT

%%

<INITIAL>{
  "/*" BEGIN(COMMENT);
}
<COMMENT>"*/" {
  BEGIN(INITIAL);
}
<COMMENT>"\n" {
  pplineno++;
}
<COMMENT>. {
  // ignore
}

"int"      return tINT;
"char"     return tCHAR;
"void"     return tVOID;
"if"       return tIF;
"else"     return tELSE;
"return"   return tRETURN;
"sizeof"   return tSIZEOF;
"++"       return tINC_OP;
"--"       return tDEC_OP;
"=="       return tEQ_OP;
"!="       return tNE_OP;
"<="       return tLE_OP;
">="       return tGE_OP;
"&&"       return tAND_OP;
"||"       return tOR_OP;
"<<"       return tLEFT_OP;
">>"       return tRIGHT_OP;
">>="      return tRIGHT_ASSIGN;
"<<="      return tLEFT_ASSIGN;
"+="       return tADD_ASSIGN;
"-="       return tSUB_ASSIGN;
"*="       return tMUL_ASSIGN;
"/="       return tDIV_ASSIGN;
"%="       return tMOD_ASSIGN;
"&="       return tAND_ASSIGN;
"^="       return tXOR_ASSIGN;
"|="       return tOR_ASSIGN;

"defined" {
  defined_mode = true;
  return tDEFINED;
}

[a-zA-Z_][a-zA-Z0-9_]* {
  if (defined_mode) {
    defined_mode = false;
    pplval.id = strdup(pptext);
    return tIDENTIFIER;
  } else {
    char *macro = find_macro(yytext);
    if (!macro) {
      pplval.id = strdup(pptext);
      return tIDENTIFIER;
    }
    if (strlen(macro) > 0 && !pp_push_string(macro)) {
      yyterminate();
    }
  }
}
<<EOF>> {
  if (!pp_pop_string()) {
    yyterminate();
  }
}

[1-9][0-9]* {
  pplval.ival = atoi(pptext);
  return tINTEGER;
}

(u|U|L)?'[^']' {
  // FIXME: u|U|L is not considered, `+` is removed and {ES} is not lexed
  char* str = read_string_literal(pptext);
  pplval.ival = str[0];
  return tINTEGER;
}

0 {
  // TODO: support all octal digits
  pplval.ival = 0;
  return tINTEGER;
}

\"([^"]|\\\")*\" {
  // TODO: change to lex ({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+
  pplval.id = read_string_literal(pptext);
  return tSTRING_LITERAL;
}

\/\/[^\n]* {
  // ignore
}

" "|\t|\r|\n {
  // ignore
}

. {
  return pptext[0];
}

%%

// To avoid compilation failure for some environments
int
ppwrap()
{
  return 1;
}

bool
pp_push_string(char *str)
{
  FILE *str_stream = fmemopen(str, strlen(str), "r");
  if (!str_stream) {
    fprintf(stderr, "Failed to fmemopen: '%s'\n", str);
    return false;
  }

  struct pp_buffer_node *new_node = malloc(sizeof(struct pp_buffer_node));
  *new_node = (struct pp_buffer_node){
    .file   = str_stream,
    .parent = pp_buffer_stack,
    .buf    = YY_CURRENT_BUFFER,
  };
  pp_buffer_stack = new_node;

  pp_switch_to_buffer(pp_create_buffer(str_stream, strlen(str)));
  return true;
}

bool
pp_pop_string()
{
  if (pp_buffer_stack == NULL) return false;

  fclose(pp_buffer_stack->file);
  yy_switch_to_buffer(pp_buffer_stack->buf);

  pp_buffer_stack = pp_buffer_stack->parent;
  return true;
}
