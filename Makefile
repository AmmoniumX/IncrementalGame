# Compiler
CXX = g++
CPPSTD = gnu++26 # Recommended: gnu++26, minimum: c++23

# Common Flags
CXXFLAGS = -Wall -Wextra -Werror -I/usr/include -std=$(CPPSTD) -fno-trapping-math -march=native

# Libraries
LDFLAGS = -lncurses -lboost_thread

# Source and Header Files
CORE_HDRS = $(wildcard src/*.hpp)
RENDER_HDRS = $(wildcard src/render/*.hpp)
SCREENS_HDRS = $(wildcard src/screens/*.hpp)
WINDOWS_HDRS = $(wildcard src/windows/*.hpp)
RESOURCES_HDRS = $(wildcard src/resources/*.hpp)
HDRS = $(CORE_HDRS) $(RENDER_HDRS) $(SCREENS_HDRS) $(WINDOWS_HDRS) $(RESOURCES_HDRS)
SRCS = $(wildcard src/*.cpp)

# Default: build release
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

# --- Phony Targets ---
all: debug release
clean:
	rm -rf ./obj/ ./bin/

.PHONY: all clean debug release
