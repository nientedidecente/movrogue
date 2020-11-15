MOVCC = movfuscator/build/movcc
CC = gcc

all: prebuild
	$(MOVCC) main.c -o main.out
	$(MOVCC) main.c -S -o main.asm

cc: prebuild
	$(CC) -std=c89 -pedantic -Wall -Wextra -Wno-char-subscripts -Wno-overlength-strings -Werror -g -O0 main.c -o main_cc.out

prebuild:
	if [[ -f "$(MOVCC)" ]]; then \
		cd movfuscator;\
		./build.sh;\
		cd -; \
	fi
