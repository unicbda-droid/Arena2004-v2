# Arena2004 - OpenGL Arena FPS + UE3-style Editor + NAT Lobby
# ---------------------------------------------------------
CC       ?= cc
PKG_CONFIG ?= pkg-config
CFLAGS   ?= -O2
CFLAGS   += -std=c11 -Wall -Wextra -Isrc -MMD -MP $(shell $(PKG_CONFIG) --cflags sdl2 glew)
UNAME_S  := $(shell uname -s 2>/dev/null)
ifeq ($(UNAME_S),Darwin)
  LDLIBS += $(shell $(PKG_CONFIG) --libs sdl2 glew) -lpng -lm -framework OpenGL
  CFLAGS += -D_DARWIN_C_SOURCE
else
  LDLIBS += $(shell $(PKG_CONFIG) --libs sdl2 glew) -lGL -lpng -lm -ldl
endif

GAME_SRC := src/gl_util.c src/ui.c src/world.c src/render.c src/game.c \
            src/hud.c src/audio.c src/editor.c src/net.c src/console.c src/steam_bridge.c \
            src/main.c src/obj_loader.c
GAME_OBJ := $(GAME_SRC:src/%.c=build/%.o)
GAME_DEP := $(GAME_OBJ:.o=.d)
LOB_OBJ  := build/lobby.o
LOB_DEP  := build/lobby.d

all: arena2004 arena2004_lobby

arena2004: $(GAME_OBJ)
	$(CC) $(CFLAGS) -o $@ $(GAME_OBJ) $(LDLIBS)

arena2004_lobby: build/lobby.o
	$(CC) $(CFLAGS) -o $@ build/lobby.o

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# Debug build with AddressSanitizer / UBSan (requires full rebuild)
debug:
	$(MAKE) clean
	$(MAKE) CFLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer" \
	        LDLIBS="$(shell $(PKG_CONFIG) --libs sdl2 glew) -lGL -lpng -lm -fsanitize=address,undefined"

run: arena2004
	./arena2004

selftest: arena2004
	./arena2004 --selftest

lobby: arena2004_lobby
	./arena2004_lobby 7777

clean:
	rm -rf build arena2004 arena2004_lobby

-include $(GAME_DEP) $(LOB_DEP)

.PHONY: all debug run selftest lobby clean
