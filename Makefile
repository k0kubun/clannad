CC=clang
CFLAGS=-Werror `llvm-config --cflags` -I./src
LD=clang++
LDFLAGS=`llvm-config --cxxflags --ldflags --libs core executionengine jit interpreter analysis native bitwriter --system-libs`
OBJS=src/debug.o src/dict.o src/main.o src/vector.o src/compiler.o src/optimizer.o src/parser.tab.o src/lex.yy.o

all: compile

compile: clannad
	cat input.c | ./clannad

run: compile
	llvm-dis main.bc && llvm-link main.ll -S -o src/linked.ll && llc src/linked.ll && gcc src/linked.s
	./a.out

clean:
	git check-ignore **/* * | xargs rm

test: clannad test/all_test.bin
	test/all_test.bin

clannad: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $@
	#----------------------------------------------------------------------------------

test/all_test.bin: test/all_test.bc test/test_helper.bc
	llvm-link test/all_test.bc test/test_helper.bc -o test/all_test.ll && llc test/all_test.ll && gcc test/all_test.s -o test/all_test.bin

test/all_test.bc: test/all_test.c clannad
	cat $< | ./clannad
	mv main.bc $@

test/test_helper.bc: test/test_helper.c clannad
	cat $< | ./clannad
	mv main.bc $@

src/debug.o: src/debug.c src/clannad.h
	$(CC) $(CFLAGS) -c $< -o $@

src/dict.o: src/dict.c src/clannad.h
	$(CC) $(CFLAGS) -c $< -o $@

src/main.o: src/main.c src/clannad.h
	$(CC) $(CFLAGS) -c $< -o $@

src/vector.o: src/vector.c src/clannad.h
	$(CC) $(CFLAGS) -c $< -o $@

src/compiler.o: src/compiler.c src/clannad.h
	$(CC) $(CFLAGS) -c $< -o $@

src/optimizer.o: src/optimizer.c src/clannad.h
	$(CC) $(CFLAGS) -c $< -o $@

src/parser.tab.o: src/parser.y src/clannad.h
	bison -dv --defines=./src/parser.tab.h -o ./src/parser.tab.c $<
	$(CC) $(CFLAGS) -c src/parser.tab.c -o $@

src/lex.yy.o: src/lexer.l src/clannad.h
	flex -o src/lex.yy.c $<
	$(CC) $(CFLAGS) -c src/lex.yy.c -o $@

.PHONY: all compile run clean test
