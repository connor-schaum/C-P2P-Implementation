#ifndef P2P_MESSAGE_H
#define P2P_MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Basic message structure
typedef struct {
    char type[32];
    char sender[64];
    char data[256];
} P2PMessage;

// Discovery message structure
typedef struct {
    char type[32];           // "DISCOVERY"
    char sender[64];         // Node ID
    int ttl;                // Time to live
    char peer_list[1024];   // Comma-separated list of peer addresses
} DiscoveryMessage;

// Message handler function type
typedef void (*message_handler_t)(P2PMessage* msg);

// Create a new message
P2PMessage* p2p_message_create(const char* type, const char* sender, const char* data);

// Default message handler
void p2p_message_default_handler(P2PMessage* msg);

// Free message
void p2p_message_free(P2PMessage* msg);

#endif
