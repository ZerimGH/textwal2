SRCS = ./src/*.c
OUT_DIR = ./build
OUT = $(OUT_DIR)/main

TOML_DIR = ./tomlc17

CC = gcc
CFLAGS = -Wall -Wextra -pedantic
CINCLUDES = -I./tomlc17/src/ -I./freetype/include -I./stb
CLINKS = -lfreetype -lm

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OUT): $(SRCS) $(OUT_DIR)
	$(CC) $(SRCS) -o $(OUT) $(CFLAGS) $(CINCLUDES) $(CLINKS)

all: $(OUT)

run: $(OUT)
	$(OUT)

clean: 
	rm -rf $(OUT_DIR)

.PHONY: all
