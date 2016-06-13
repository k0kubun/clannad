CC=clang
CFLAGS=-Werror `llvm-config --cflags` -I./src
LD=clang++
LDFLAGS=`llvm-config --cxxflags --ldflags --libs core executionengine jit interpreter analysis native bitwriter --system-libs`

all: compile

compile: clannad
	cat input.c | ./clannad

run: compile
	llvm-dis main.bc && llvm-link main.ll -S -o sample/linked.ll && llc sample/linked.ll && gcc sample/linked.s
	./a.out

clannad: tmp tmp/debug.o tmp/main.o tmp/vector.o tmp/parser.tab.o tmp/lex.yy.o
	$(LD) tmp/debug.o tmp/main.o tmp/vector.o tmp/parser.tab.o tmp/lex.yy.o $(LDFLAGS) -o ./clannad
	#----------------------------------------------------------------------------------

tmp:
	mkdir -p tmp

tmp/debug.o: src/clannad.h src/debug.c
	$(CC) $(CFLAGS) -c src/debug.c -o tmp/debug.o

tmp/main.o: src/clannad.h src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o tmp/main.o

tmp/vector.o: src/clannad.h src/vector.c
	$(CC) $(CFLAGS) -c src/vector.c -o tmp/vector.o

tmp/parser.tab.o: src/clannad.h src/parser.y
	bison -dv --defines=./tmp/parser.tab.h -o ./tmp/parser.tab.c src/parser.y
	$(CC) $(CFLAGS) -c tmp/parser.tab.c -o tmp/parser.tab.o

tmp/lex.yy.o: src/clannad.h tmp/parser.tab.o src/lexer.l
	flex -o tmp/lex.yy.c src/lexer.l
	$(CC) $(CFLAGS) -c tmp/lex.yy.c -o tmp/lex.yy.o
