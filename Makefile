CC=clang
CLND=./clannad
CFLAGS=-Werror `llvm-config --cflags` -I./src -DCLANG_VERSION="\"`clang --version | head -n1 | cut -d' ' -f3`\""
LD=clang++
LDFLAGS=`llvm-config --cxxflags --ldflags --libs core executionengine jit interpreter analysis native bitwriter --system-libs`
OBJS=src/analyzer.o src/assembler.o src/compiler.o src/debug.o src/dict.o src/lex.pp.o src/lex.yy.o \
     src/main.o src/optimizer.o src/parser.tab.o src/preprocessor.tab.o src/vector.o
SRCS := $(patsubst %.o,%.c,$(OBJS))
TESTS := $(patsubst %.c,%.bin,$(wildcard test/*.c))
TESTOBJS := $(patsubst %.c,%.o,$(wildcard test/*.c))
.PHONY: all run clean test self
.SECONDARY: $(TESTOBJS)

all: clannad

clannad: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $@

self: clannad
	@for src in $$(echo $(SRCS)); do \
		echo "===[Compiling $$src...]======================" ; \
		($(CLND) $$src && echo "*** Succeeded! ***") || echo "--- Failed. ---"; \
		echo ; \
	done

run: clannad
	$(CLND) input.c && cc input.o && ./a.out

ast: clannad
	$(CLND) -fdump-ast input.c

ir1: clannad
	$(CLND) -fdump-ir1 input.c

ir2: clannad
	$(CLND) -fdump-ir2 input.c

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

test/%.bin: test/%.o
	cc $< -o $@

test/%.o: test/%.c clannad
	$(CLND) $<

src/parser.tab.h: src/parser.tab.c
src/parser.tab.c: src/parser.y
	bison -dv --defines=./src/parser.tab.h -o $@ $<

src/preprocessor.tab.h: src/preprocessor.tab.c
src/preprocessor.tab.c: src/preprocessor.y
	bison -dv --name-prefix=pp --defines=./src/preprocessor.tab.h -o $@ $<

src/lex.yy.c: src/lexer.l src/parser.tab.h
	flex -o $@ $<

src/lex.pp.c: src/preprocessor.l src/preprocessor.tab.h
	flex -o $@ --prefix=pp $<
