%{
#include "clannad.h"
#include "parser.tab.h"
%}

%%

"int" return tINT;

[a-zA-Z_][a-zA-Z0-9_]* {
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
