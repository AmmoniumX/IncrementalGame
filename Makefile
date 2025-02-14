# Compiler Flags
CXX = g++
CXXFLAGS = -Wall -Wextra -I/usr/include -std=c++23

# Target-dependent Flags
DEBUGFLAGS = -g -O0
OPTIMIZATIONFLAGS = -O3

# Libraries
LDFLAGS = -lncurses

# Target and Source Files
TARGET = bin/game
OBJS = game.o
SRCS = game.cpp
HEADERS = $(wildcard headers/*.hh)
SCREENS = $(wildcard screens/*.hh)

# Build Rules
all: $(TARGET)

bin/game: game.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

game.o: game.cpp $(HEADERS) $(SCREENS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Phony Targets
clean:
	rm -f $(OBJS) $(TARGET)

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: all

release: CXXFLAGS += $(OPTIMIZATIONFLAGS)
release: all

.PHONY: all clean debug release
