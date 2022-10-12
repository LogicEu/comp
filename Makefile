# comp Makefile

CC=gcc
SRC=main.c
STD=-std=c89
WFLAGS=-Wall -Wextra
EXE=comp

SCRIPT=build.sh

CFLAGS= $(STD) $(WFLAGS) $(INC)

$(EXE): $(SRC)
	$(CC) -o $@ $^ $(CFLAGS)

clean: $(SCRIPT)
	./$^ $@

install: $(SCRIPT)
	./$^ $@

uninstall: $(SCRIPT)
	./$^ $@./$^ $@
