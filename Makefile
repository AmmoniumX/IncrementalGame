# Compiler Flags
CXX = g++
CXXFLAGS = -Wall -Wextra -I/usr/include

# Target-dependent Flags
DEBUGFLAGS = -g -O0
OPTIMIZATIONFLAGS = -O3

# Libraries
LDFLAGS = -lgmp -lgmpxx -lncurses

# Target and Source Files
TARGET = bin/game
OBJS = game.o
SRCS = game.cpp

# Build Rules
all: $(TARGET)

bin/game: game.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

game.o: game.cpp headers/game.hh headers/json.hh headers/render.hh
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Phony Targets
clean:
	rm -f $(OBJS) $(TARGET)

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: 
	$(MAKE) clean
	$(MAKE) all

release: CXXFLAGS += $(OPTIMIZATIONFLAGS)
release:
	$(MAKE) clean
	$(MAKE) all

.PHONY: all clean debug release
