# Compiler
CXX = g++

# C++ Standard (Recommended: gnu++26, minimum: c++23)
CXXSTD = gnu++26

# Common Flags
CXXFLAGS = -Wall -Wextra -Werror -std=$(CXXSTD) -march=native

# Set to enable certain constexpr optimizations
CXXFLAGS += -fno-trapping-math -DNO_TRAPPING_MATH

# Libraries
EXTRA_INCLUDES ?=
EXTRA_LDFLAGS ?=

INCLUDES = 
ifeq ($(shell uname -o),Msys)
    CXXFLAGS += -DPDCURSES_WCS -DPDC_WIDE -lstdc++exp
    LDFLAGS = -static -l:wincon/pdcurses.a
else
    LDFLAGS = -lncursesw
endif
INCLUDES += $(EXTRA_INCLUDES)
LDFLAGS += $(EXTRA_LDFLAGS)

SRCS = $(shell find src -name '*.cpp')

# Default: build release
BUILD_TYPE ?= release

ifeq ($(BUILD_TYPE),release)
    # --- Release Build ---
    BIN_DIR = bin/release
    OBJ_DIR = obj/release
    ifeq ($(shell uname -o),Msys)
      TARGET = $(BIN_DIR)/game.exe
    else
      TARGET = $(BIN_DIR)/game
    endif
    OBJS = $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
    CXXFLAGS += -O3

else ifeq ($(BUILD_TYPE),debug)
    # --- Debug Build ---
    BIN_DIR = bin/debug
    OBJ_DIR = obj/debug
    ifeq ($(shell uname -o),Msys)
      TARGET = $(BIN_DIR)/game.exe
      CXXFLAGS += -g -O0
    else
      TARGET = $(BIN_DIR)/game
      CXXFLAGS += -g -O0 -fsanitize=address,undefined
    endif
    OBJS = $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
endif

ifeq ($(shell uname -o),Msys)
    OBJS += $(OBJ_DIR)/render/wcwidth.o
endif

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS)

$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# --- Phony Targets ---
all: debug release
clean:
	rm -rf ./obj/* ./bin/*

debug:
	$(MAKE) BUILD_TYPE=debug

release:
	$(MAKE) BUILD_TYPE=release

.PHONY: all clean debug release
