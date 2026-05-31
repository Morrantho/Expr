CC=clang
STD=-std=c2y
SRC=main.c
LIB=
WARN=-Wall -Wextra -Werror -Wpedantic
NOWARN=-Wno-unused-parameter -Wno-gnu-label-as-value -Wno-gnu-designator -Wno-initializer-overrides
DBG=-O0 -ggdb3 -fno-omit-frame-pointer -DDEBUG
REL=-O3 -flto -march=native -DNDEBUG
FAST=-O3 -flto -ffast-math -fomit-frame-pointer -fno-stack-protector -march=native -DNDEBUG -s
ASM=-O3 -ffast-math -march=native -S
.PHONY: debug release fast asm

debug:
	$(CC) $(STD) $(WARN) $(NOWARN) $(DBG) $(SRC) $(LIB) -o a

release:
	$(CC) $(STD) $(WARN) $(NOWARN) $(REL) $(SRC) $(LIB) -o a

fast:
	$(CC) $(STD) $(WARN) $(NOWARN) $(FAST) $(SRC) $(LIB) -o a

asm:
	$(CC) $(STD) $(WARN) $(NOWARN) $(ASM) $(SRC) $(LIB) -o a.s