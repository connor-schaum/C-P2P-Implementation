# C-P2P-Implementation

A peer-to-peer network implementation in C that provides automatic peer discovery, mesh topology formation, and persistent peer management.

## Features

- **Automatic Peer Discovery**: Nodes automatically discover and connect to each other
- **Mesh Topology**: Creates a true peer-to-peer network where each node connects to multiple peers
- **Persistent Peer Lists**: Peer information is saved to files for bootstrapping on restart
- **Discovery Propagation**: TTL-based discovery messages spread through the network
- **Duplicate Prevention**: File-based duplicate checking prevents redundant connections
- **Auto-Connection**: Nodes automatically connect to newly discovered peers

## Architecture

### Core Components

- **`p2p_main.c`**: Main application entry point
- **`p2p_network.c`**: Network layer handling connections and discovery
- **`p2p_peer.c`**: Peer management and persistence
- **`p2p_message.c`**: Message handling and structures
- **`p2p_utils.c`**: Utility functions

### Message Types

- **`P2PMessage`**: General messages with type, sender, and data
- **`DiscoveryMessage`**: Specialized discovery messages with TTL and peer lists

## Building

```bash
make
```

This will compile the executable `p2p_main`.

## Usage

### Basic Usage

```bash
# Start first node
./p2p_main --address 127.0.0.1:1248

# Start second node (connects to first)
./p2p_main --address 127.0.0.1:1249 --connect-to 127.0.0.1:1248

# Start third node (connects to second)
./p2p_main --address 127.0.0.1:1250 --connect-to 127.0.0.1:1249
```

### Commands

Once a node is running, you can use these commands:

- **`send <address> <type> <data>`**: Send a message to a specific peer
- **`broadcast <type> <data>`**: Broadcast a message to all peers
- **`list`**: List all known peers
- **`quit`**: Exit the node

### Example Session

```bash
$ ./p2p_main --address 127.0.0.1:1248
Starting P2P node on 127.0.0.1:1248
P2P Node ready.
Commands: 'send <address> <type> <data>', 'broadcast <type> <data>', 'list', 'quit'
> list
127.0.0.1:1249
127.0.0.1:1250
> broadcast MESSAGE "Hello from node 1248"
Broadcast MESSAGE to 2 peers
> quit
```

## How It Works

### 1. Node Startup
- Node loads existing peers from file (if any)
- Automatically connects to known peers (bootstrap)
- Starts server thread to accept incoming connections

### 2. Peer Discovery
- When a node connects, it sends discovery messages to all its peers
- Discovery messages contain the node's peer list and TTL
- TTL decrements as discovery propagates through the network

### 3. Auto-Connection
- When a node receives discovery, it automatically connects to new peers
- Creates a mesh topology where each node connects to multiple peers
- Prevents duplicate connections through file-based checking

### 4. Persistence
- Each node maintains a peer list file: `127.0.0.1:PORT_PeerList.txt`
- Peer information persists across restarts
- Nodes can bootstrap from their saved peer lists

## Network Topology

The system creates a mesh network where:
- Each node connects to multiple peers
- Discovery messages propagate through the network
- Nodes automatically learn about each other
- No single point of failure

## File Structure

```
C-P2P-Implementation/
├── p2p_main.c              # Main application
├── p2p_network.c           # Network layer
├── p2p_peer.c              # Peer management
├── p2p_message.c           # Message handling
├── p2p_utils.c             # Utility functions
├── p2p_*.h                 # Header files
├── DataStructures/         # Custom data structures
│   ├── Lists/LinkedList.c
│   └── Common/Node.c
├── Makefile                # Build configuration
└── README.md               # This file
```

## Dependencies

- **GCC**: C compiler
- **pthread**: POSIX threads library
- **Standard C libraries**: stdio, stdlib, string, socket, etc.

## Building from Source

```bash
# Clone the repository
git clone <repository-url>
cd C-P2P-Implementation

# Build the project
make

# Clean build artifacts
make clean
```

## Example Network Formation

1. **NodeA** starts (isolated)
2. **NodeB** connects to **NodeA** → both learn about each other
3. **NodeC** connects to **NodeA** → NodeA learns about NodeC
4. **NodeA** forwards discovery to **NodeB** → NodeB learns about NodeC
5. **NodeB** auto-connects to **NodeC** → true mesh network!

## Troubleshooting

### Common Issues

- **Port already in use**: Choose a different port number
- **Connection refused**: Ensure the target node is running
- **Discovery not propagating**: Check that nodes are connecting to different peers

### Debug Output

The system provides detailed debug output showing:
- Peer connections and disconnections
- Discovery message propagation
- TTL values and forwarding
- Auto-connection attempts

## License

This project is open source. See the repository for license details.

## Future Enhancements

- Peer health checking and reconnection
- Message encryption and authentication
- Distributed consensus mechanisms
- Performance optimizations
- Network monitoring and metrics
