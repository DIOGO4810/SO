CC = gcc
CFLAGS := -Wall -Wno-unused-result -g -Og -O2 -Wextra -pedantic -Iinclude $(shell pkg-config --cflags glib-2.0) 
LDFLAGS := $(shell pkg-config --libs glib-2.0) -L/usr/lib/x86_64-linux-gnu -lm

SRC_UTILS := $(shell find src/utils/ -name "*.c")
SRC_CLIENT_UTILS := $(filter-out src/utils/serverUtils.c src/utils/persistencia.c, $(shell find src/utils/ -name "*.c"))


SRC_DSERVER := src/dserver.c $(SRC_UTILS)
SRC_DCLIENT := src/dclient.c $(SRC_CLIENT_UTILS)

OBJ_DSERVER := $(SRC_DSERVER:src/%.c=obj/%.o)
OBJ_DCLIENT := $(SRC_DCLIENT:src/%.c=obj/%.o)

all: folders dserver dclient

dserver: bin/dserver
dclient: bin/dclient

folders:
	@mkdir -p src include obj bin tmp
	@mkdir -p obj/utils

# Compilação do dclient
bin/dclient: $(OBJ_DCLIENT)
	@$(CC) $(OBJ_DCLIENT) $(LDFLAGS) -o $@

# Compilação do dserver
bin/dserver: $(OBJ_DSERVER)
	@$(CC) $(OBJ_DSERVER) $(LDFLAGS) -o $@

# Compilação dos arquivos .c para .o
obj/%.o: src/%.c
	@mkdir -p $(dir $@)  
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj/* tmp/* bin/* indexs ordem
