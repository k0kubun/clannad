CC=clang
CFLAGS=-Werror `llvm-config --cflags` -I./src
LD=clang++
LDFLAGS=`llvm-config --cxxflags --ldflags --libs core executionengine jit interpreter analysis native bitwriter --system-libs`
OBJS=tmp/debug.o tmp/dict.o tmp/main.o tmp/vector.o tmp/compiler.o tmp/optimizer.o tmp/parser.tab.o tmp/lex.yy.o

all: compile

compile: clannad
	cat input.c | ./clannad

run: compile
	llvm-dis main.bc && llvm-link main.ll -S -o tmp/linked.ll && llc tmp/linked.ll && gcc tmp/linked.s
	./a.out

clean:
	git check-ignore **/* * | xargs rm

test: clannad test/all_test.bin
	test/all_test.bin

clannad: tmp $(OBJS)
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

tmp:
	mkdir -p tmp

tmp/debug.o: src/debug.c src/clannad.h
	$(CC) $(CFLAGS) -c $< -o $@

tmp/dict.o: src/dict.c src/clannad.h
	$(CC) $(CFLAGS) -c $< -o $@

tmp/main.o: src/main.c src/clannad.h
	$(CC) $(CFLAGS) -c $< -o $@

tmp/vector.o: src/vector.c src/clannad.h
	$(CC) $(CFLAGS) -c $< -o $@

tmp/compiler.o: src/compiler.c src/clannad.h
	$(CC) $(CFLAGS) -c $< -o $@

tmp/optimizer.o: src/optimizer.c src/clannad.h
	$(CC) $(CFLAGS) -c $< -o $@

tmp/parser.tab.o: src/parser.y src/clannad.h
	bison -dv --defines=./tmp/parser.tab.h -o ./tmp/parser.tab.c $<
	$(CC) $(CFLAGS) -c tmp/parser.tab.c -o $@

tmp/lex.yy.o: src/lexer.l src/clannad.h
	flex -o tmp/lex.yy.c $<
	$(CC) $(CFLAGS) -c tmp/lex.yy.c -o $@
