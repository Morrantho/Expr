CC=cc
STD=-std=gnu23
SRC=src/main.c
LIB=
WARN=-Wall -Wextra -Werror -Wpedantic -Winline
NOWARN=-Wno-unused-function -Wno-unused-parameter
DBG=-O0 -ggdb3 -fno-omit-frame-pointer -DDEBUG
FAST=-Ofast -flto -fwhole-program -fomit-frame-pointer -fno-stack-protector -march=native -DNDEBUG
ASM=-masm=intel -Ofast -fwhole-program -fomit-frame-pointer -fno-stack-protector -march=native -S

debug:
	$(CC) $(STD) $(WARN) $(NOWARN) $(DBG) $(SRC) $(LIB)

fast:
	$(CC) $(STD) $(WARN) $(NOWARN) $(FAST) $(SRC) $(LIB)

asm:
	$(CC) $(STD) $(WARN) $(NOWARN) $(ASM) $(SRC) $(LIB) -o a.s

calls:
	@objdump -drwC -Mintel ./a.out | awk "/call/ { print }"

clean:
	rm -f a.out a.s