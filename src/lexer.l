%{
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include "clannad.h"
#include "parser.tab.h"

char* copy_string(char *str);
char* read_string_literal(char *str);
bool push_file(char *filename, bool std);
bool pop_file();

struct buffer_node {
  struct buffer_node *parent;
  YY_BUFFER_STATE buf;
  char *filename;
  FILE *file;
};

static struct buffer_node *buffer_stack = NULL;
static Vector *std_search_paths = NULL;
static char* compile_dir = NULL;
static char* compile_file = NULL;
%}

%s COMMENT
%x INCLUDE_FILE

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

^"#"[ \t]*include[ \t]* {
  BEGIN(INCLUDE_FILE);
}
<INCLUDE_FILE>[\"<][^ \t\n\">]+ {
  bool std = false;
  int c = input();
  switch (yytext[0]) {
    case '<':
      std = true;
      if (c != '>') {
        fprintf(stderr, "error: missing terminating > character\n");
        yyterminate();
      }
      break;
    case '"':
      if (c != '"') {
        fprintf(stderr, "error: missing terminating \" character\n");
        yyterminate();
      }
      break;
  }
  while ((c = input()) && c != '\n');

  if (!push_file(strdup(yytext + 1), std)) {
    yyterminate();
  }

  BEGIN(INITIAL);
}
<INCLUDE_FILE>.|\n {
  fprintf(stderr, "Bad #include is found.\n");
  yyterminate();
}
<<EOF>> {
  if (!pop_file()) {
    yyterminate();
  }
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
  yylval.id = copy_string(yytext);
  return tIDENTIFIER;
}

[1-9][0-9]* {
  yylval.ival = atoi(yytext);
  return tINTEGER;
}

(u|U|L)?'[^']' {
  // FIXME: u|U|L is not considered, `+` is removed and {ES} is not lexed
  char* str = read_string_literal(yytext);
  yylval.ival = str[0];
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

\/\/[^\n]* {
  // ignore
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
    case 'a':
      return '\a';
    case 'b':
      return '\b';
    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
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

void
init_search_paths()
{
  std_search_paths = create_vector();
  vector_push(std_search_paths, "/usr/include/linux");
  vector_push(std_search_paths, "/usr/include");
  vector_push(std_search_paths, "/usr/local/include");
}

void
set_compile_path(char* filename)
{
  compile_file = filename;
  compile_dir  = dirname(strdup(filename));
}

char*
get_reading_file()
{
  if (buffer_stack) {
    return buffer_stack->filename;
  } else {
    return compile_file;
  }
}

FILE*
try_include(char *filename, bool std)
{
  FILE *file;
  char *dir, path[256];
  if (!std && compile_dir) {
    sprintf(path, "%s/%s", compile_dir, filename);
    file = fopen(path, "r");
    if (file) return file;
  }

  for (int i = 0; i < std_search_paths->length; i++) {
    dir = vector_get(std_search_paths, i);
    sprintf(path, "%s/%s", dir, filename);
    file = fopen(path, "r");
    if (file) break;
  }
  return file;
}

bool
push_file(char *filename, bool std)
{
  FILE *file = try_include(filename, std);
  if (!file) {
    fprintf(stderr, "Failed to include: '%s'\n", filename);
    return false;
  }

  struct buffer_node *new_node = malloc(sizeof(struct buffer_node));
  *new_node = (struct buffer_node){
    .file     = file,
    .filename = filename,
    .parent   = buffer_stack,
    .buf      = YY_CURRENT_BUFFER,
  };
  buffer_stack = new_node;

  yy_switch_to_buffer(yy_create_buffer(file, YY_BUF_SIZE));
  return true;
}

bool
pop_file()
{
  if (buffer_stack == NULL) return false;

  fclose(buffer_stack->file);
  yy_switch_to_buffer(buffer_stack->buf);

  buffer_stack = buffer_stack->parent;
  return true;
}
