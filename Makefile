CC=clang
CFLAGS=-Werror `llvm-config --cflags` -I./src
LD=clang++
LDFLAGS=`llvm-config --cxxflags --ldflags --libs core executionengine jit interpreter analysis native bitwriter --system-libs`
OBJS=src/assembler.o src/debug.o src/dict.o src/main.o src/vector.o src/compiler.o src/optimizer.o src/parser.tab.o src/lex.yy.o
TESTS=test/test2.bin test/all_test.bin
.PHONY: all compile run clean test

all: compile

compile: clannad
	./clannad input.c && gcc input.o

run: compile
	./a.out

clean:
	git check-ignore **/* * | xargs rm

test: clannad $(TESTS)
	@for test in $(TESTS); do \
		./$$test || exit; \
	done

clannad: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $@

test/all_test.bin: test/all_test.o test/test_helper.o
	gcc $< test/test_helper.o -o $@

test/test2.bin: test/test2.o test/test_helper.o
	gcc $< test/test_helper.o -o $@

test/all_test.o: test/all_test.c clannad
	./clannad $<

test/test2.o: test/test2.c clannad
	./clannad $<

test/test_helper.o: test/test_helper.c clannad
	./clannad $<

$(OBJS): src/clannad.h

src/parser.tab.c: src/parser.y
	bison -dv --defines=./src/parser.tab.h -o $@ $<

src/lex.yy.c: src/lexer.l
	flex -o $@ $<
