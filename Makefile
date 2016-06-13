CC=clang
CFLAGS=-Werror `llvm-config --cflags`
LD=clang++
LDFLAGS=`llvm-config --cxxflags --ldflags --libs core executionengine jit interpreter analysis native bitwriter --system-libs`

all: clannad

clannad: src/main.o
	$(LD) $< $(LDFLAGS) -o $@

src/main.o: src/main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf ../**/*.o ../**/*.ll ../**/*.s ../**/*.out ../**/*.bc

compile: clannad
	./clannad && llvm-dis main.bc && cat main.ll

# Current status
run: compile
	llvm-link main.ll -S -o sample/linked.ll && llc sample/linked.ll && gcc sample/linked.s
	./a.out

# Goal
test: sample/test.c
	clang -emit-llvm -S -O -o sample/test.ll sample/test.c
	llvm-link sample/test.ll -S -o sample/linked.ll && llc sample/linked.ll && gcc sample/linked.s
	./a.out
