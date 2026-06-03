CC=cc
STD=-std=gnu23
SRC=src/unity.c
EXCLUDES=-not -name "unity.c" -not -name "main.c"
SRCS=$(shell find src/ -name "*.c" -type f $(EXCLUDES))
LIB=-lm
WARN=-Wall -Wextra -Werror -Wpedantic
NOWARN=-Wno-unused-parameter
DBG=-O0 -ggdb3 -fno-omit-frame-pointer -DDEBUG
REL=-O3 -flto -march=native -DNDEBUG
FAST=-Ofast -flto -fomit-frame-pointer -fno-stack-protector -march=native -DNDEBUG -s
ASM=-Ofast -march=native -S
.PHONY: debug release fast asm

debug:
	$(CC) $(STD) $(WARN) $(NOWARN) $(DBG) $(SRC) $(LIB)

release:
	$(CC) $(STD) $(WARN) $(NOWARN) $(REL) $(SRC) $(LIB)

fast:
	$(CC) $(STD) $(WARN) $(NOWARN) $(FAST) $(SRC) $(LIB)

obj:
	$(CC) $(STD) $(WARN) $(NOWARN) $(FAST) $(SRCS) $(LIB)

asm:
	$(CC) $(STD) $(WARN) $(NOWARN) $(ASM) $(SRC) $(LIB) -o a.s

clean:
	rm -f a.out a.s