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

## Core Components

- **`p2p_main.c`**: Main application entry point with command-line parsing and interactive command loop
- **`p2p_network.c`**: Network layer handling TCP connections, server thread, and discovery mechanisms
- **`p2p_peer.c`**: Peer management with file-based persistence
- **`p2p_message.c`**: Message handling and structures
- **`p2p_utils.c`**: Utility functions for peer list string building

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
