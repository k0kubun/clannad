CC=clang
CLND=./clannad
CFLAGS=-Werror `llvm-config --cflags` -I./src
LD=clang++
LDFLAGS=`llvm-config --cxxflags --ldflags --libs core executionengine jit interpreter analysis native bitwriter --system-libs`
OBJS=src/analyzer.o src/assembler.o src/compiler.o src/debug.o src/dict.o src/lex.yy.o \
     src/main.o src/optimizer.o src/parser.tab.o src/vector.o
TESTS := $(patsubst %.c,%.bin,$(filter-out test/test_helper.c,$(wildcard test/*.c)))
TESTOBJS := $(patsubst %.c,%.o,$(wildcard test/*.c))
.PHONY: all run clean test
.SECONDARY: $(TESTOBJS)

all: clannad

clannad: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $@

run: clannad
	$(CLND) input.c && gcc input.o && ./a.out

ast: clannad
	$(CLND) -fdump-ast input.c

clean:
	for file in $$(git check-ignore **/* * | grep -v input.c); do \
		rm $$file ; \
	done

$(OBJS): src/clannad.h

test: clannad $(TESTS)
	@for test in $(TESTS); do \
		echo $$test: ; \
		./$$test || exit; \
		echo && echo; \
	done
	@echo All tests have been passed!
	@echo

test/%.bin: test/%.o test/test_helper.o
	gcc $< test/test_helper.o -o $@

test/%.o: test/%.c clannad
	$(CLND) $<

src/parser.tab.h: src/parser.tab.c

src/parser.tab.c: src/parser.y
	bison -dv --defines=./src/parser.tab.h -o $@ $<

src/lex.yy.c: src/lexer.l src/parser.tab.h
	flex -o $@ $<
