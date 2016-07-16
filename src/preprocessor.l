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
  Dict *macros;
};
static struct pp_buffer_node *pp_buffer_stack = NULL;

char* read_string_literal(char *str);
static bool defined_mode = false;
bool pp_push_string(char *str);
bool pp_pop_string();
int pp_peek_token();
Vector* pp_read_macro_args();
%}
O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
HP  (0[xX])
E   ([Ee][+-]?{D}+)
P   ([Pp][+-]?{D}+)
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP  (u|U|L)
SP  (u8|u|U|L)
ES  (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS  [ \t\v\n\f]

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

{L}{A}* {
  if (defined_mode) {
    defined_mode = false;
    pplval.id = strdup(pptext);
    return tIDENTIFIER;
  } else {
    Macro *macro;
    if ((macro = find_macro(pptext)) && (!macro->is_function || pp_peek_token() == '(')) {
      if (macro->is_function) {
        Vector *args = pp_read_macro_args();
        if (macro->params->length != args->length) {
          fprintf(stderr, "Macro argument count mismatch!\n");
          yyterminate();
        }
        if (strlen(macro->value) > 0 && !pp_push_string(macro->value)) {
          yyterminate();
        } else {
          Dict *subst = build_macro_subst(macro, args);
          push_macros(subst);
        }
      } else {
        if (strlen(macro->value) > 0 && !pp_push_string(macro->value)) yyterminate();
      }
    } else {
      pplval.id = strdup(pptext);
      return tIDENTIFIER;
    }
  }
}
<<EOF>> {
  if (!pp_pop_string()) {
    yyterminate();
  }
}

{HP}{H}+{IS}? {
  pplval.ival = atoi(pptext);
  return tI_CONSTANT;
}
{NZ}{D}*{IS}? {
  pplval.ival = atoi(pptext);
  return tI_CONSTANT;
}
"0"{O}*{IS}? {
  pplval.ival = atoi(pptext);
  return tI_CONSTANT;
}
{CP}?"'"([^'\\\n]|{ES})+"'" {
  pplval.ival = atoi(pptext);
  return tI_CONSTANT;
}

{D}+{E}{FS}? {
  pplval.fval = atof(pptext);
  return tF_CONSTANT;
}
{D}*"."{D}+{E}?{FS}? {
  pplval.fval = atof(pptext);
  return tF_CONSTANT;
}
{D}+"."{E}?{FS}? {
  pplval.fval = atof(pptext);
  return tF_CONSTANT;
}
{HP}{H}+{P}{FS}? {
  pplval.fval = atof(pptext);
  return tF_CONSTANT;
}
{HP}{H}*"."{H}+{P}{FS}? {
  pplval.fval = atof(pptext);
  return tF_CONSTANT;
}
{HP}{H}+"."{P}{FS}? {
  pplval.fval = atof(pptext);
  return tF_CONSTANT;
}

(u|U|L)?'[^']' {
  // FIXME: u|U|L is not considered, `+` is removed and {ES} is not lexed
  char* str = read_string_literal(pptext);
  pplval.ival = str[0];
  return tI_CONSTANT;
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
    .macros = check_macros(),
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

  set_macros(pp_buffer_stack->macros);
  pp_buffer_stack = pp_buffer_stack->parent;
  return true;
}

int
pp_peek_token()
{
  int ret = pplex();
  for (int i = strlen(pptext) - 1; 0 <= i; i--) {
    unput(pptext[i]);
  }
  return ret;
}

Vector*
pp_read_macro_args()
{
  Vector *ret = create_vector();
  char *buf = malloc(sizeof(char));
  buf[0] = '\0';

  int depth = 0, token;
  for (;;) {
    switch ((token = pplex())) {
      case ',':
        if (depth == 1) {
          vector_push(ret, buf);
          buf = malloc(sizeof(char));
          buf[0] = '\0';
          continue;
        }
        break;
      case '(':
        depth++;
        if (depth == 1) continue;
        break;
      case ')':
        depth--;
        if (depth == 0) {
          if (strlen(buf) > 0) vector_push(ret, buf);
          return ret;
        }
        break;
    }

    buf = realloc(buf, (strlen(buf) + strlen(pptext)) * sizeof(char));
    strcat(buf, pptext);
  }
  return ret;
}
