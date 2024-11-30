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

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

# Clean Rule
clean:
	rm -f $(TARGET)
