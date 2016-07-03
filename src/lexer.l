%{
#include "clannad.h"
#include "parser.tab.h"

char* copy_string(char *str);
char* read_string_literal(char *str);
%}

%s COMMENT

%%

<INITIAL>{
  "/*" BEGIN(COMMENT);
}
<COMMENT>{
  "*/"    BEGIN(INITIAL);
  [^*\n]+ // eat comment in chunks
  "*"     // eat the lone star
  \n      yylineno++;
}

"int"    return tINT;
"char"   return tCHAR;
"void"   return tVOID;
"if"     return tIF;
"else"   return tELSE;
"return" return tRETURN;
"sizeof" return tSIZEOF;
"++"     return tINC_OP;
"--"     return tDEC_OP;
"=="     return tEQ_OP;
"!="     return tNE_OP;
"<="     return tLE_OP;
">="     return tGE_OP;

[a-zA-Z_][a-zA-Z0-9_]* {
  yylval.id = copy_string(yytext);
  return tIDENTIFIER;
}

-?[1-9][0-9]* {
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
  yylval.id = read_string_literal(yytext);
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

char
read_escaped_char(char c)
{
  switch (c) {
    case 'n':
      return '\n';
    default:
      fprintf(stderr, "unexpected escape sequence: '\\%c'", c);
      exit(1);
  }
}

char*
read_string_literal(char *str)
{
  char *ret = (char *)malloc(sizeof(char) * (strlen(str) - 1));

  int pos = 0;
  for (int i = 1; i < strlen(str) - 1; i++) { // Ignoreing `"`.
    char c = str[i];
    if (c == '\\') {
      c = read_escaped_char(str[++i]);
    }
    ret[pos++] = c;
  }
  ret[pos] = '\0';
  return ret;
}
