CC=cc
STD=-std=gnu23
SRC=src/main.c
LIB=
WARN=-Wall -Wextra -Werror -Wpedantic -Winline
NOWARN=-Wno-unused-function
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

asm:
	$(CC) $(STD) $(WARN) $(NOWARN) $(ASM) $(SRC) $(LIB) -o a.s

calls:
	@objdump -drwC -Mintel ./a.out | awk "/call/ { print }"

clean:
	rm -f a.out a.s