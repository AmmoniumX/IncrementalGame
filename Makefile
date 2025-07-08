# Compiler Flags
CXX = g++
CXXFLAGS = -Wall -Wextra -I/usr/include -std=c++26 -fno-trapping-math -march=native -fsanitize=address,undefined

# Target-dependent Flags
DEBUGFLAGS = -g -O0
OPTIMIZATIONFLAGS = -O3

# Libraries
LDFLAGS = -lncurses

# Target and Source Files
TARGET = bin/game

CORE_HDRS = $(wildcard core/*.hpp)
SCREENS_HDRS = $(wildcard screens/*.hpp)
RESOURCES_HDRS = $(wildcard resources/*.hpp)

CORE_SRCS = $(wildcard core/*.cpp)
SCREENS_SRCS = $(wildcard screens/*.cpp)
RESOURCES_SRCS = $(wildcard resources/*.cpp)

SRCS = $(CORE_SRCS) $(SCREENS_SRCS) $(RESOURCES_SRCS)
OBJS = $(SRCS:.cpp=.o)


# Build Rules
all: $(TARGET)

bin/game: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

core/game.o: core/game.cpp $(CORE_HDRS) $(SCREENS_HDRS) $(RESOURCES_HDRS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Phony Targets
clean:
	rm -f $(OBJS) $(TARGET)

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: all

release: CXXFLAGS += $(OPTIMIZATIONFLAGS)
release: all

.PHONY: all clean debug release
