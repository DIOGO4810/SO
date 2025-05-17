CC = gcc
CFLAGS := -Wall -Wno-unused-result -g -Og -O2 -Wextra -pedantic -Iinclude $(shell pkg-config --cflags glib-2.0)
LDFLAGS := $(shell pkg-config --libs glib-2.0) -L/usr/lib/x86_64-linux-gnu -lm

# Encontrar todos os arquivos .c em utils e serverUtils
SRC_UTILS := $(shell find src/utils/ -name "*.c")
SRC_SERVER_UTILS := $(shell find src/serverUtils/ -name "*.c")
SRC_INDEX := src/index.c  # Agora index é um arquivo único

# Definir os arquivos fonte para dserver (incluindo os necessários de utils, serverUtils e index)
SRC_DSERVER := src/dserver.c

# Definir os arquivos fonte para dclient (incluindo os necessários de utils e index)
SRC_DCLIENT := src/dclient.c

# Definir os arquivos objeto para utils, serverUtils, index, dserver e dclient
OBJ_UTILS := $(SRC_UTILS:src/%.c=obj/%.o)
OBJ_SERVER_UTILS := $(SRC_SERVER_UTILS:src/%.c=obj/%.o)
OBJ_INDEX := obj/index.o  # Corrigido para o arquivo objeto
OBJ_DSERVER := $(SRC_DSERVER:src/%.c=obj/%.o)
OBJ_DCLIENT := $(SRC_DCLIENT:src/%.c=obj/%.o)

# Alvos principais
all: folders dserver dclient

# Compilação do dclient
dclient: bin/dclient

# Compilação do dserver
dserver: bin/dserver

# Criar os diretórios necessários
folders:
	@mkdir -p src include obj bin tmp
	@mkdir -p obj/utils
	@mkdir -p obj/serverUtils

# Compilação do dclient (usa objetos de utils, index)
bin/dclient: $(OBJ_DCLIENT) $(OBJ_UTILS) $(OBJ_INDEX)
	@$(CC) $(OBJ_DCLIENT) $(OBJ_UTILS) $(OBJ_INDEX) $(LDFLAGS) -o $@

# Compilação do dserver (usa objetos de utils, serverUtils, index)
bin/dserver: $(OBJ_DSERVER) $(OBJ_UTILS) $(OBJ_SERVER_UTILS) $(OBJ_INDEX)
	@$(CC) $(OBJ_DSERVER) $(OBJ_UTILS) $(OBJ_SERVER_UTILS) $(OBJ_INDEX) $(LDFLAGS) -o $@

# Compilação dos arquivos .c para .o
obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj/* tmp/* bin/dclient bin/dserver indexs ordem

recompile:
	rm -rf obj/* bin/dclient bin/dserver