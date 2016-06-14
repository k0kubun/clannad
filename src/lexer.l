%{
#include "clannad.h"
#include "parser.tab.h"

char* copy_string(char *str);
%}

%%

"int" return tINT;
"return" return tRETURN;

[a-zA-Z_][a-zA-Z0-9_]* {
  yylval.id = copy_string(yytext);
  return tIDENTIFIER;
}

[1-9][0-9]* {
  yylval.ival = atoi(yytext);
  return tINTEGER;
}

0 {
  // TODO: support all octal digits
  yylval.ival = 0;
  return tINTEGER;
}

\"([^"]|\\\")*\" {
  // TODO: change to lex ({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+
  int len = strlen(yytext);
  yylval.id = (char *)malloc(sizeof(char) * (len - 1));
  strcpy(yylval.id, yytext + 1);
  yylval.id[len - 2] = '\0';
  return tSTRING_LITERAL;
}

" "|\t|\r|\n {
  // ignore
}

. {
  return yytext[0];
}

%%

// To avoid compilation failure for some environments
int
yywrap()
{
  return 1;
}

char*
copy_string(char *str)
{
  char *ret = (char *)malloc(sizeof(char) * (strlen(str) + 1));
  strcpy(ret, str);
  return ret;
}
