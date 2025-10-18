#ifndef P2P_NETWORK_H
#define P2P_NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "p2p_message.h"
#include "p2p_peer.h"

// Network configuration
typedef struct {
    int port;
    char node_id[64];
    P2PPeerList* peer_list;
    message_handler_t message_handler;
} P2PNetwork;

// Create network
P2PNetwork* p2p_network_create(int port, const char* node_id, message_handler_t handler);

// Start network (starts server thread)
int p2p_network_start(P2PNetwork* network);

// Send message to specific address
int p2p_network_send(P2PNetwork* network, const char* address, const char* type, const char* data);

// Broadcast message to all peers
int p2p_network_broadcast(P2PNetwork* network, const char* type, const char* data);

// Connect to a peer
int p2p_network_connect(P2PNetwork* network, const char* address);

// Stop network
void p2p_network_stop(P2PNetwork* network);

// Free network
void p2p_network_free(P2PNetwork* network);

#endif
