# C-P2P-Implementation

A peer-to-peer network implementation in C that provides automatic peer discovery, mesh topology formation, and persistent peer management using TCP sockets.

## Features

- **TCP-Based Networking**: Uses TCP sockets for reliable peer-to-peer communication
- **Automatic Peer Discovery**: Nodes automatically discover and connect to each other
- **Mesh Topology**: Creates a true peer-to-peer network where each node connects to multiple peers
- **Persistent Peer Lists**: Peer information is saved to files for bootstrapping on restart
- **Discovery Propagation**: TTL-based discovery messages spread through the network (initial TTL=3)
- **Duplicate Prevention**: File-based duplicate checking prevents redundant connections
- **Auto-Connection**: Nodes automatically connect to newly discovered peers
- **Bootstrap Support**: Nodes automatically connect to known peers from saved files on startup

## Architecture

### Core Components

- **`p2p_main.c`**: Main application entry point with command-line parsing and interactive command loop
- **`p2p_network.c`**: Network layer handling TCP connections, server thread, and discovery mechanisms
- **`p2p_peer.c`**: Peer management with file-based persistence
- **`p2p_message.c`**: Message handling and structures
- **`p2p_utils.c`**: Utility functions for peer list string building

### Message Types

- **`P2PMessage`**: General messages with type (32 chars), sender (64 chars), and data (256 chars)
- **`DiscoveryMessage`**: Specialized discovery messages with TTL and comma-separated peer lists (1024 chars)

### Data Structures

The implementation uses a custom LinkedList data structure from the `DataStructures/` directory:
- `DataStructures/Lists/LinkedList.c` - Dynamic linked list implementation
- `DataStructures/Common/Node.c` - Node structure for linked lists

## Building

```bash
make
```

This will compile the executable `p2p_main`.

To clean build artifacts:
```bash
make clean
```

## Usage

### Basic Usage

```bash
# Start first node (default address: 127.0.0.1:1248)
./p2p_main --address 127.0.0.1:1248

# Start second node (connects to first)
./p2p_main --address 127.0.0.1:1249 --connect-to 127.0.0.1:1248

# Start third node (connects to second)
./p2p_main --address 127.0.0.1:1250 --connect-to 127.0.0.1:1249
```

### Command-Line Arguments

- **`--address <IP:PORT>`**: Sets the address and port for this node (required, default: `127.0.0.1:1248`)
- **`--connect-to <IP:PORT>`**: Optional initial peer to connect to on startup

### Interactive Commands

Once a node is running, you can use these commands:

- **`send <address> <type> <data>`**: Send a message to a specific peer
  - Example: `send 127.0.0.1:1249 MESSAGE "Hello from node 1248"`
- **`broadcast <type> <data>`**: Broadcast a message to all known peers
  - Example: `broadcast MESSAGE "Hello everyone"`
- **`list`**: List all known peers with their addresses
- **`quit`**: Exit the node gracefully

### Example Session

```bash
$ ./p2p_main --address 127.0.0.1:1248
Starting P2P node on 127.0.0.1:1248
Server running on port 1248
P2P Node ready.
Commands: 'send <address> <type> <data>', 'broadcast <type> <data>', 'list', 'quit'
> list
Known peers (2):
  0: 127.0.0.1:1249
  1: 127.0.0.1:1250
> broadcast MESSAGE "Hello from node 1248"
Broadcast MESSAGE to 2 peers
Sent MESSAGE to 127.0.0.1:1249
Sent MESSAGE to 127.0.0.1:1250
Received MESSAGE from 127.0.0.1:1249: Hello from node 1248
Received MESSAGE from 127.0.0.1:1250: Hello from node 1248
> quit
```

## How It Works

### 1. Node Startup

- Node extracts port from the `--address` argument
- Creates a P2P network instance with the specified port and address
- Loads existing peers from file: `{node_id}_PeerList.txt` (e.g., `127.0.0.1:1248_PeerList.txt`)
- Automatically connects to all known peers from the file (bootstrap)
- Starts server thread to accept incoming TCP connections

### 2. Peer Discovery

- When a node connects to a peer or receives a connection, it sends discovery messages to all its peers
- Discovery messages contain:
  - Type: "DISCOVERY"
  - Sender: Node's address
  - TTL: Time-to-live (initial value 3)
  - Peer list: Comma-separated list of all known peer addresses
- TTL decrements as discovery propagates through the network
- When TTL reaches 0, the message is not forwarded further

### 3. Auto-Connection

- When a node receives a discovery message:
  - It adds the sender to its peer list
  - It parses the peer list and adds new peers
  - If a peer is newly added, it automatically connects to that peer
  - It responds with its own peer list (with decremented TTL)
  - It forwards the discovery to all other peers (propagation)

### 4. Persistence

- Each node maintains a peer list file: `{node_id}_PeerList.txt`
- Peer addresses are stored one per line
- File is checked for duplicates before adding new peers
- Peer information persists across restarts
- Nodes bootstrap from their saved peer lists on startup

### 5. Message Handling

- Regular messages (non-DISCOVERY) are handled by the message handler callback
- Default handler prints received messages to stdout
- Messages are sent via TCP connections that are opened for each send operation
- Server thread handles incoming connections and routes messages appropriately

## Network Topology

The system creates a mesh network where:
- Each node maintains TCP connections (opened on-demand for messages)
- Discovery messages propagate through the network with TTL limiting
- Nodes automatically learn about each other through discovery
- Multiple paths between nodes provide redundancy
- No single point of failure

## Dependencies

- **GCC**: C compiler
- **pthread**: POSIX threads library (for server thread)
- **Standard C libraries**: stdio, stdlib, string, time
- **POSIX socket libraries**: socket, arpa/inet, netinet/in (for TCP networking)

## Building from Source

```bash
# Build the project
make

# Run a node
./p2p_main --address 127.0.0.1:1248

# Clean build artifacts
make clean
```

## Example Network Formation

1. **NodeA** (127.0.0.1:1248) starts → isolated, starts server thread
2. **NodeB** (127.0.0.1:1249) connects to **NodeA** → both learn about each other via discovery
3. **NodeC** (127.0.0.1:1250) connects to **NodeA** → NodeA learns about NodeC, sends discovery to NodeB
4. **NodeB** receives discovery from **NodeA** containing NodeC → NodeB learns about NodeC
5. **NodeB** auto-connects to **NodeC** → all nodes know each other → mesh network!

## Troubleshooting

### Common Issues

- **Port already in use**: Choose a different port number
- **Connection refused**: Ensure the target node is running and accepting connections
- **Discovery not propagating**: Check that nodes are connecting and discovery TTL is sufficient
- **File permission errors**: Ensure write permissions for creating peer list files

### Debug Output

The system provides debug output showing:
- Server startup on specified port
- Peer connections and bootstrap attempts
- Discovery message sending and receiving
- TTL values and forwarding decisions
- Auto-connection attempts
- Message sending/receiving

## Technical Details

- **Protocol**: TCP
- **Default Port**: 1248 (if --address not specified)
- **Discovery TTL**: 3 (initial value)
- **Peer List Format**: Plain text file, one address per line
- **Message Format**: Binary structures sent over TCP
- **Concurrency**: Server thread handles incoming connections asynchronously

## Future Enhancements

- Message encryption and authentication
- Distributed consensus mechanisms
- Performance optimizations
- Network monitoring and metrics
- Peer list cleanup and expiration

## License

This project is open source. See the repository for license details.
