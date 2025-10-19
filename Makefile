# P2P Node Makefile

CC = gcc
CFLAGS = -I. -lpthread -Wall

# Target executable
TARGET = p2p_main

# Source files
SOURCES = p2p_main.c p2p_message.c p2p_peer.c p2p_network.c p2p_utils.c

# Dependencies (DataStructures)
DEPS = DataStructures/Lists/LinkedList.c \
       DataStructures/Common/Node.c

# Build target
all: $(TARGET)

# Build P2P main executable
$(TARGET): $(SOURCES) $(DEPS)
	$(CC) -o $(TARGET) $(SOURCES) $(DEPS) $(CFLAGS)

# Clean build artifacts
clean:
	rm -f $(TARGET)

.PHONY: all clean