CC=clang
STD=-std=c2y
SRC=src/main.c
LIB=-lm
WARN=-Wall -Wextra -Werror -Wpedantic
NOWARN=-Wno-gnu-designator -Wno-initializer-overrides
DBG=-O0 -ggdb3 -fno-omit-frame-pointer -DDEBUG
REL=-O3 -flto -march=native -DNDEBUG
FAST=-O3 -flto -ffast-math -fomit-frame-pointer -fno-stack-protector -march=native -DNDEBUG -s
ASM=-O3 -ffast-math -march=native -S
.PHONY: debug release fast asm

debug:
	$(CC) $(STD) $(WARN) $(NOWARN) $(DBG) $(SRC) $(LIB)

release:
	$(CC) $(STD) $(WARN) $(NOWARN) $(REL) $(SRC) $(LIB)

fast:
	$(CC) $(STD) $(WARN) $(NOWARN) $(FAST) $(SRC) $(LIB)

asm:
	$(CC) $(STD) $(WARN) $(NOWARN) $(ASM) $(SRC) $(LIB) -o a.s

clean:
	rm -f a.out a.s