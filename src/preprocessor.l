%{
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include "clannad.h"
#include "preprocessor.tab.h"

char* read_string_literal(char *str);
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
"const"    return tCONST;
"volatile" return tVOLATILE;
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

[a-zA-Z_][a-zA-Z0-9_]* {
  pplval.id = strdup(pptext);
  return tIDENTIFIER;
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
