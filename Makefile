BIN := bin/gltf_viewer

CC := gcc
CFLAGS := -std=gnu99
IFLAGS := -Iinclude/ -Isrc/ -Ilibs/rebound/ -Ilibs/glfw/include/ -Ilibs/glad/include/ -Ilibs/HandmadeMath/
LFLAGS := libs/rebound/rebound.o -Llibs/glfw/src/ -lglfw3 libs/glad/glad.o -lm
DFLAGS :=

debug: CFLAGS += -ggdb -Wall -Wextra -MD -MP
debug: DFLAGS += -DRE_DEBUG
release: CFLAGS += -O3

SRC := $(wildcard src/*.c) $(wildcard src/**/*.c) $(wildcard src/**/**/*.c)
VPATH := $(dir $(SRC))

OBJ := $(patsubst src/%,obj/%,$(SRC:%.c=%.o))

DEP := $(OBJ:%.o=%.d)
-include $(DEP)

.DEFAULT_GOAL := debug

debug: build
release: clean build

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(IFLAGS) $(DFLAGS)

build: make_dirs build_libs $(OBJ)
	@mkdir -p $(dir $(BIN))
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN) $(LFLAGS)

make_dirs:
	@mkdir -p $(dir $(OBJ))

# Libraries
libs/rebound/rebound.o: libs/rebound/rebound.c
	$(CC) $(CFLAGS) -c libs/rebound/rebound.c -o libs/rebound/rebound.o $(DFLAGS)

libs/glfw/src/libglfw3.a:
	cd libs/glfw && cmake . && make

libs/glad/glad.o: libs/glad/src/gl.c
	$(CC) -O3 -c libs/glad/src/gl.c -o libs/glad/glad.o -Ilibs/glad/include

build_libs: libs/rebound/rebound.o libs/glfw/src/libglfw3.a libs/glad/glad.o

.PHONY: clean
clean:
	rm -f $(OBJ)
	rm -f $(DEP)
	rm -rf obj/
	rm -f $(BIN)
