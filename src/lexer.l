%{
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include "clannad.h"
#include "parser.tab.h"

char* read_string_literal(char *str);
void set_reading_file(char *filename);
bool is_defined(char *identifier);
bool push_file(char *filename, bool std);
bool push_string(char *str);
bool pop_file();

// Linked list to keep YY_BUFFER_STATE during `yy_switch_to_buffer` calls.
struct buffer_node {
  struct buffer_node *parent;
  YY_BUFFER_STATE buf;
  char *filename;
  FILE *file;
};
static struct buffer_node *buffer_stack = NULL;

// Linked list to keep state number (`yy_start`) to use after `#endif` token.
struct state_node {
  struct state_node *parent;
  int state;
};
static struct state_node *state_stack = NULL;

static Vector *std_search_paths = NULL;
static Dict *define_rules = NULL;
static char* compile_dir  = NULL;
static char* compile_file = NULL;
static char* define_from  = NULL;
%}

%x COMMENT
%x DEFINE_FROM
%x DEFINE_TO
%x UNDEF
%x INCLUDE
%x IF
%x IFDEF
%x IFNDEF
%x IGNORED
%x LINE_NUM
%x LINE_FILE

%%

<INITIAL>{
  "/*" BEGIN(COMMENT);
}
<COMMENT>"*/" {
  BEGIN(INITIAL);
}
<COMMENT>"\n" {
  yylineno++;
}
<COMMENT>. {
  // ignore
}

^"#"[ \t]*define[ \t]* {
  BEGIN(DEFINE_FROM);
}
<DEFINE_FROM>[a-zA-Z_][a-zA-Z0-9_]* {
  define_from = strdup(yytext);
  BEGIN(DEFINE_TO);
}
<DEFINE_TO>[^\n]*$ {
  if (!define_rules) define_rules = create_dict();
  dict_set(define_rules, define_from, strdup(yytext));
  BEGIN(INITIAL);
}

^"#"[ \t]*undef[ \t]* {
  BEGIN(UNDEF);
}
<UNDEF>[a-zA-Z_][a-zA-Z0-9_]* {
  if (!define_rules) define_rules = create_dict();
  dict_delete(define_rules, yytext);
  BEGIN(INITIAL);
}

^"#"[ \t]*ifdef[ \t]+ {
  struct state_node *new_node = malloc(sizeof(struct state_node));
  new_node->state  = YYSTATE;
  new_node->parent = state_stack;
  state_stack = new_node;
  BEGIN(IFDEF);
}
<IFDEF>[a-zA-Z_][a-zA-Z0-9_]* {
  if (is_defined(yytext)) {
    BEGIN(INITIAL);
  } else {
    BEGIN(IGNORED);
  }
}

^"#"[ \t]*if[ \t]+ {
  struct state_node *new_node = malloc(sizeof(struct state_node));
  new_node->state  = YYSTATE;
  new_node->parent = state_stack;
  state_stack = new_node;
  BEGIN(IF);
}
<IF>[^\n]+ {
  Node *ast;
  if (pp_parse_exp(&ast, strdup(yytext))) {
    fprintf(stderr, "Preprocessor parse error!\n");
    exit(1);
    return 1;
  }
  BEGIN(INITIAL);
}

^"#"[ \t]*ifndef[ \t]+ {
  struct state_node *new_node = malloc(sizeof(struct state_node));
  new_node->state  = YYSTATE;
  new_node->parent = state_stack;
  state_stack = new_node;
  BEGIN(IFNDEF);
}
<IFNDEF>[a-zA-Z_][a-zA-Z0-9_]* {
  if (is_defined(yytext)) {
    BEGIN(IGNORED);
  } else {
    BEGIN(INITIAL);
  }
}

<INITIAL,IGNORED>^"#"[ \t]*endif[ \t]* {
  if (state_stack == NULL) {
    fprintf(stderr, "error: #endif without #if\n");
    exit(1);
  }
  BEGIN(state_stack->state);
  state_stack = state_stack->parent;
}
<IGNORED>"\n" {
  yylineno++;
}
<IGNORED>. {
  // ignore
}

^"#"[ \t]*include[ \t]* {
  BEGIN(INCLUDE);
}
<INCLUDE>[\"<][^ \t\n\">]+ {
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
<INCLUDE>.|\n {
  fprintf(stderr, "Bad #include is found.\n");
  yyterminate();
}
<<EOF>> {
  if (!pop_file()) {
    if (state_stack) {
      fprintf(stderr, "error: unterminated #if, #ifdef or #ifndef.\n");
      exit(1);
    }
    yyterminate();
  }
}

^"#"[ \t]*line[ \t]+ {
  BEGIN(LINE_NUM);
}
<LINE_NUM>[1-9][0-9]* {
  yylineno = atoi(yytext);
  BEGIN(LINE_FILE);
}
<LINE_FILE>[ \t]+\"[^\"]+\" {
  char *filename = strdup(yytext);
  filename[strlen(filename)-1] = '\0';

  int i;
  for (i = 0; filename[i++] != '"';);
  set_reading_file(filename + i);

  BEGIN(INITIAL);
}
<LINE_FILE>[ \t]*"\n" {
  BEGIN(INITIAL);
}
<LINE_FILE>. {
  fprintf(stderr, "error: unexpected character after #line\n");
  yyterminate();
}

^"#"[ \t]*[a-z]+ {
  fprintf(stderr, "error: unexpected preprocessor: %s\n", yytext);
  yyterminate();
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
  char *macro;
  if (define_rules && (macro = dict_get(define_rules, yytext))) {
    if (strlen(macro) > 0 && !push_string(macro)) {
      yyterminate();
    }
  } else {
    yylval.id = strdup(yytext);
    return tIDENTIFIER;
  }
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
  vector_push(std_search_paths, "/usr/local/include");
  vector_push(std_search_paths, "/usr/lib/clang/" CLANG_VERSION "/include");
  vector_push(std_search_paths, "/usr/local/lib/clang/" CLANG_VERSION "/include");
  vector_push(std_search_paths, "/usr/include");
}

void
set_compile_path(char* filename)
{
  compile_file = filename;
  compile_dir  = dirname(strdup(filename));
}

void
set_reading_file(char *filename)
{
  if (buffer_stack) {
    buffer_stack->filename = filename;
  } else {
    compile_file = filename;
  }
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
is_defined(char *identifier)
{
  return define_rules && dict_get(define_rules, identifier);
}

void
push_fp(FILE *fp, int size, char *filename)
{
  struct buffer_node *new_node = malloc(sizeof(struct buffer_node));
  *new_node = (struct buffer_node){
    .file     = fp,
    .filename = filename,
    .parent   = buffer_stack,
    .buf      = YY_CURRENT_BUFFER,
  };
  buffer_stack = new_node;

  yy_switch_to_buffer(yy_create_buffer(fp, size));
}

bool
push_file(char *filename, bool std)
{
  FILE *file = try_include(filename, std);
  if (!file) {
    fprintf(stderr, "Failed to include: '%s'\n", filename);
    return false;
  }

  push_fp(file, YY_BUF_SIZE, filename);
  return true;
}

bool
push_string(char *str)
{
  FILE *str_stream = fmemopen(str, strlen(str), "r");
  if (!str_stream) {
    fprintf(stderr, "Failed to fmemopen: '%s'\n", str);
    return false;
  }

  push_fp(str_stream, strlen(str), "(define)");
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
