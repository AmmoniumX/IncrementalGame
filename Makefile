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
INCLUDES= -I/usr/include $(EXTRA_INCLUDES)
LDFLAGS = -lncursesw $(EXTRA_LDFLAGS)

SRCS = $(shell find src -name '*.cpp')

# Default: build release
BUILD_TYPE ?= release

ifeq ($(BUILD_TYPE),release)
    # --- Release Build ---
    BIN_DIR = bin/release
    OBJ_DIR = obj/release
    TARGET = $(BIN_DIR)/game
	OBJS = $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
    CXXFLAGS += -O3 -fsanitize=address,undefined

else ifeq ($(BUILD_TYPE),debug)
    # --- Debug Build ---
    BIN_DIR = bin/debug
    OBJ_DIR = obj/debug
    TARGET = $(BIN_DIR)/game
	OBJS = $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
    CXXFLAGS += -g -O0
endif

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS)

$(OBJ_DIR)/%.o: src/%.cpp src/%.hpp
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
