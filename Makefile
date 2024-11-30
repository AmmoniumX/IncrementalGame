# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I/usr/include

# Libraries
LDFLAGS = -lgmp -lgmpxx -lncurses

# Target and Source Files
TARGET = bin/game
SRCS = game.cpp

# Build Rules
all: $(TARGET)

bin/game: game.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

game.o: game.cpp headers/game.hpp headers/json.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean Rule
clean:
	rm -f $(TARGET)
