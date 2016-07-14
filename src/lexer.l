%{
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include "clannad.h"
#include "parser.tab.h"

char* read_string_literal(char *str);
void set_reading_file(char *filename);
bool pp_eval(Node *node);
bool is_defined(char *identifier);
bool push_file(char *filename, bool std);
bool push_string(char *str);
bool pop_file();
void push_state();

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
  bool had_true;
};
static struct state_node *state_stack = NULL;

static Vector *std_search_paths = NULL;
static Dict *define_rules = NULL;
static char* compile_dir  = NULL;
static char* compile_file = NULL;
static char* prev_yytext  = NULL;
bool is_ignored(struct state_node *node);
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
%x DEFINE_KEY
%x DEFINE_VAR
%x UNDEF
%x INCLUDE
%x IF
%x IFDEF
%x IFNDEF
%x ELIF
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
  BEGIN(DEFINE_KEY);
}
<DEFINE_KEY>{L}{A}* {
  prev_yytext = strdup(yytext);
  BEGIN(DEFINE_VAR);
}
<DEFINE_VAR>[^\n]*$ {
  if (!define_rules) define_rules = create_dict();
  dict_set(define_rules, prev_yytext, strdup(yytext));
  BEGIN(INITIAL);
}

^"#"[ \t]*undef[ \t]* {
  BEGIN(UNDEF);
}
<UNDEF>{L}{A}* {
  if (!define_rules) define_rules = create_dict();
  dict_delete(define_rules, yytext);
  BEGIN(INITIAL);
}

<INITIAL,IGNORED>^"#"[ \t]*ifdef[ \t]+ {
  push_state();
  BEGIN(IFDEF);
}
<IFDEF>{L}{A}* {
  if (is_defined(yytext)) {
    state_stack->had_true = true;
    BEGIN(INITIAL);
  } else {
    BEGIN(IGNORED);
  }
}

<INITIAL,IGNORED>^"#"[ \t]*if[ \t]+ {
  push_state();
  BEGIN(IF);
}
<IF>[^\n]+ {
  Node *ast;
  if (pp_parse_exp(&ast, yytext)) {
    fprintf(stderr, "Preprocessor failed to parse '%s'!\n", yytext);
    yyterminate();
  }

  if (!is_ignored(state_stack) && pp_eval(ast)) {
    state_stack->had_true = true;
    BEGIN(INITIAL);
  } else {
    BEGIN(IGNORED);
  }
}

<INITIAL,IGNORED>^"#"[ \t]*ifndef[ \t]+ {
  push_state();
  BEGIN(IFNDEF);
}
<IFNDEF>{L}{A}* {
  if (is_defined(yytext)) {
    BEGIN(IGNORED);
  } else {
    state_stack->had_true = true;
    BEGIN(INITIAL);
  }
}

<INITIAL,IGNORED>^"#"[ \t]*elif[ \t]+ {
  if (state_stack == NULL) {
    fprintf(stderr, "error: #endif without #if\n");
    yyterminate();
  }
  BEGIN(ELIF);
}
<ELIF>[^\n]+ {
  Node *ast;
  if (pp_parse_exp(&ast, yytext)) {
    fprintf(stderr, "Preprocessor failed to parse '%s'!\n", yytext);
    yyterminate();
  }

  if (!state_stack->had_true && !is_ignored(state_stack) && pp_eval(ast)) {
    state_stack->had_true = true;
    BEGIN(INITIAL);
  } else {
    BEGIN(IGNORED);
  }
}

<INITIAL,IGNORED>^"#"[ \t]*endif[ \t]* {
  if (state_stack == NULL) {
    fprintf(stderr, "error: #endif without #if\n");
    yyterminate();
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
"float"    return tFLOAT;
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

{L}{A}* {
  char *macro;
  if ((macro = find_macro(yytext))) {
    if (strlen(macro) > 0 && !push_string(macro)) {
      yyterminate();
    }
  } else {
    yylval.id = strdup(yytext);
    return tIDENTIFIER;
  }
}

{HP}{H}+{IS}? {
  yylval.ival = atoi(yytext);
  return tI_CONSTANT;
}
{NZ}{D}*{IS}? {
  yylval.ival = atoi(yytext);
  return tI_CONSTANT;
}
"0"{O}*{IS}? {
  yylval.ival = atoi(yytext);
  return tI_CONSTANT;
}
{CP}?"'"([^'\\\n]|{ES})+"'" {
  yylval.ival = atoi(yytext);
  return tI_CONSTANT;
}

{D}+{E}{FS}? {
  yylval.fval = atof(yytext);
  return tF_CONSTANT;
}
{D}*"."{D}+{E}?{FS}? {
  yylval.fval = atof(yytext);
  return tF_CONSTANT;
}
{D}+"."{E}?{FS}? {
  yylval.fval = atof(yytext);
  return tF_CONSTANT;
}
{HP}{H}+{P}{FS}? {
  yylval.fval = atof(yytext);
  return tF_CONSTANT;
}
{HP}{H}*"."{H}+{P}{FS}? {
  yylval.fval = atof(yytext);
  return tF_CONSTANT;
}
{HP}{H}+"."{P}{FS}? {
  yylval.fval = atof(yytext);
  return tF_CONSTANT;
}

(u|U|L)?'[^']' {
  // FIXME: u|U|L is not considered, `+` is removed and {ES} is not lexed
  char* str = read_string_literal(yytext);
  yylval.ival = str[0];
  return tI_CONSTANT;
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

// Eval `#if`'s parameter
bool
pp_eval(Node *node)
{
  switch (node->kind) {
    case NODE_DEFINED:
      return is_defined(node->id);
    case NODE_UNARY:
      switch (node->op) {
        case '!':
          return !pp_eval(node->val);
        default:
          fprintf(stderr, "Unexpected unary operation in pp_eval: %c (%d)\n", node->op, node->op);
          yyterminate();
      }
    case NODE_BINOP:
      switch (node->op) {
        case '+':
          return pp_eval(node->lhs) + pp_eval(node->rhs);
        case '-':
          return pp_eval(node->lhs) - pp_eval(node->rhs);
        case '*':
          return pp_eval(node->lhs) * pp_eval(node->rhs);
        case '/':
          return pp_eval(node->lhs) / pp_eval(node->rhs);
        case '>':
          return pp_eval(node->lhs) > pp_eval(node->rhs);
        case '<':
          return pp_eval(node->lhs) < pp_eval(node->rhs);
        case GE_OP:
          return pp_eval(node->lhs) >= pp_eval(node->rhs);
        case LE_OP:
          return pp_eval(node->lhs) <= pp_eval(node->rhs);
        case AND_OP:
          return pp_eval(node->lhs) && pp_eval(node->rhs);
        case OR_OP:
          return pp_eval(node->lhs) || pp_eval(node->rhs);
        default:
          fprintf(stderr, "Unexpected binary operation in pp_eval: %c (%d)\n", node->op, node->op);
          yyterminate();
      }
      break;
    case NODE_INTEGER:
      return node->ival;
    case NODE_IDENTIFIER:
      // This should be replaced by macro if defined. Thus this flow must be unexpected.
      fprintf(stderr, "Undefined identifier referenced in preprocessor: '%s'\n", node->id);
      yyterminate();
    default:
      fprintf(stderr, "Unexpected node in pp_eval: %s\n", kind_label(node->kind));
      yyterminate();
  }
}

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
  vector_push(std_search_paths, "/usr/include/x86_64-linux-gnu");
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

  file = drop_backslash_newline(file);
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

void
push_state()
{
  struct state_node *new_node = malloc(sizeof(struct state_node));
  new_node->state  = YYSTATE;
  new_node->parent = state_stack;
  new_node->had_true = false;
  state_stack = new_node;
}

char*
find_macro(char *key)
{
  if (!define_rules) return NULL;
  return dict_get(define_rules, key);
}

bool
is_ignored(struct state_node *node)
{
  if (YYSTATE == IGNORED) return true;
  if (!node) return false;
  return node->state == IGNORED || is_ignored(node->parent);
}
