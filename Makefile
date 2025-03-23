
CC = gcc
CFLAGS = -Wall -g -Iinclude
LDFLAGS =


SRC_UTILS := $(shell find src/utils/ -name "*.c")

SRC_DSERVER := src/dserver.c $(SRC_UTILS)
SRC_DCLIENT := src/dclient.c $(SRC_UTILS)

OBJ_DSERVER := $(SRC_DSERVER:src/%.c=obj/%.o)
OBJ_DCLIENT := $(SRC_DCLIENT:src/%.c=obj/%.o)


all: folders dserver dclient

dserver: bin/dserver
dclient: bin/dclient

folders:
	@mkdir -p src include obj bin tmp
	@mkdir -p obj/utils


# Compilação do dserver
bin/dserver: $(OBJ_DSERVER)
	@$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@


# Compilação do dclient
bin/dclient: $(OBJ_DCLIENT)
	@$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@


# Compilação dos arquivos .c para .o
obj/%.o: src/%.c
	@mkdir -p $(dir $@)  
	@$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf obj/* tmp/* bin/*
