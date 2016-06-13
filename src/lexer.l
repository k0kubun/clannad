%{
#include "clannad.h"
#include "parser.tab.h"

char* copy_string(char *str);
%}

%%

"int" return tINT;

[a-zA-Z_][a-zA-Z0-9_]* {
  yylval.id = copy_string(yytext);
  return tIDENTIFIER;
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
