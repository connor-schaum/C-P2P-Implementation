# P2P Node Makefile

CC = gcc
CFLAGS = -I. -lpthread -Wall

# Target executable
TARGET = p2p_main

# Source files
SOURCES = p2p_main.c p2p_message.c p2p_peer.c p2p_network.c

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

# Test the P2P system
test: $(TARGET)
	@echo "Testing P2P system..."
	@echo "Start Node A: ./p2p_main --port 1248"
	@echo "Start Node B: ./p2p_main --port 1249 --connect-to 127.0.0.1:1248"

# Install (copy to system path)
install: $(TARGET)
	@echo "Installing P2P system..."
	@cp $(TARGET) /usr/local/bin/ 2>/dev/null || echo "Installation requires sudo"

# Uninstall
uninstall:
	@echo "Uninstalling P2P system..."
	@rm -f /usr/local/bin/$(TARGET) 2>/dev/null || echo "Uninstall requires sudo"

.PHONY: all clean test install uninstall