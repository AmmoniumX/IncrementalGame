# Compiler
CXX = g++
CPPSTD = gnu++26 # Recommended: gnu++26, minimum: c++23

# Common Flags
CXXFLAGS = -Wall -Wextra -Werror -I/usr/include -std=$(CPPSTD) -fno-trapping-math -march=native

# Libraries
LDFLAGS = -lncurses

# Source and Header Files
CORE_HDRS = $(wildcard core/*.hpp)
SCREENS_HDRS = $(wildcard screens/*.hpp)
RESOURCES_HDRS = $(wildcard resources/*.hpp)
HDRS = $(CORE_HDRS) $(SCREENS_HDRS) $(RESOURCES_HDRS)

CORE_SRCS = $(wildcard core/*.cpp)
SCREENS_SRCS = $(wildcard screens/*.cpp)
RESOURCES_SRCS = $(wildcard resources/*.cpp)
SRCS = $(CORE_SRCS) $(SCREENS_SRCS) $(RESOURCES_SRCS)

# Default: build release
all: release

# --- Debug Build ---
DEBUG_DIR = bin/debug
DEBUG_OBJ_DIR = obj/debug
DEBUG_TARGET = $(DEBUG_DIR)/game
DEBUG_OBJS = $(patsubst %.cpp,$(DEBUG_OBJ_DIR)/%.o,$(SRCS))
DEBUG_CXXFLAGS = $(CXXFLAGS) -g -O0

debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	@mkdir -p $(DEBUG_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(DEBUG_OBJ_DIR)/%.o: %.cpp $(HDRS)
	@mkdir -p $(@D)
	$(CXX) $(DEBUG_CXXFLAGS) -c -o $@ $<

# --- Release Build ---
RELEASE_DIR = bin/release
RELEASE_OBJ_DIR = obj/release
RELEASE_TARGET = $(RELEASE_DIR)/game
RELEASE_OBJS = $(patsubst %.cpp,$(RELEASE_OBJ_DIR)/%.o,$(SRCS))
RELEASE_CXXFLAGS = $(CXXFLAGS) -O3 -fsanitize=address,undefined

release: $(RELEASE_TARGET)

$(RELEASE_TARGET): $(RELEASE_OBJS)
	@mkdir -p $(RELEASE_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -fsanitize=address,undefined

$(RELEASE_OBJ_DIR)/%.o: %.cpp $(HDRS)
	@mkdir -p $(@D)
	$(CXX) $(RELEASE_CXXFLAGS) -c -o $@ $<

# --- Phony Targets ---
clean:
	rm -rf ./obj/ ./bin/

.PHONY: all clean debug release
