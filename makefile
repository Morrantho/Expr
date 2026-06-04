CC=cc
STD=-std=gnu23
SRC=src/unity.c
EXCLUDES=-not -name "unity.c" -not -name "main.c"
SRCS=$(shell find src/ -name "*.c" -type f $(EXCLUDES))
LIB=-lm
WARN=-Wall -Wextra -Werror -Wpedantic -Winline
NOWARN=
DBG=-O0 -ggdb3 -fno-omit-frame-pointer -DDEBUG
FAST=-Ofast -fomit-frame-pointer -fno-stack-protector -march=native -DNDEBUG
FASTER=-Ofast -flto -fomit-frame-pointer -fno-stack-protector -march=native -DNDEBUG
FASTEST=-Ofast -flto -fwhole-program -fomit-frame-pointer -fno-stack-protector -march=native -DNDEBUG
ASM=-Ofast -fomit-frame-pointer -fno-stack-protector -march=native -S # no LTO, or we will be blind and confused.

debug:
	$(CC) $(STD) $(WARN) $(NOWARN) $(DBG) $(SRC) $(LIB)

fast:
	$(CC) $(STD) $(WARN) $(NOWARN) $(FAST) $(SRC) $(LIB)

faster:
	$(CC) $(STD) $(WARN) $(NOWARN) $(FASTER) $(SRC) $(LIB)

fastest:
	$(CC) $(STD) $(WARN) $(NOWARN) $(FASTEST) $(SRC) $(LIB)

obj:
	$(CC) $(STD) $(WARN) $(NOWARN) $(FAST) $(SRCS) $(LIB)

asm:
	$(CC) $(STD) $(WARN) $(NOWARN) $(ASM) $(SRC) $(LIB) -o a.s

calls:
	@objdump -drwC -Mintel ./a.out | awk "/call/ { print }"

syms:
	@nm -C ./a.out | grep -E " Lex| Parse| Compile| Run| Vm| Mem| Src"

clean:
	rm -f a.out a.s