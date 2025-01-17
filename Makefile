# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I/usr/include

# Debug Flags
DEBUGFLAGS = -g -O0

# Libraries
LDFLAGS = -lgmp -lgmpxx -lncurses

# Target and Source Files
TARGET = bin/game
SRCS = game.cpp

# Build Rules
all: $(TARGET)

bin/game: game.o
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -o $@ $^ $(LDFLAGS)

game.o: game.cpp headers/game.hh headers/json.hh headers/render.hh
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c -o $@ $<

# Clean Rule
clean:
	rm -f game.o $(TARGET)

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: clean all


# Phony Targets
.PHONY: all clean debug