#ifndef P2P_MESSAGE_H
#define P2P_MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Message structure
typedef struct {
    char type[32];
    char sender[64];
    char data[256];
    int ttl;
} P2PMessage;

// Message handler function type
typedef void (*message_handler_t)(P2PMessage* msg);

// Create a new message
P2PMessage* p2p_message_create(const char* type, const char* sender, const char* data, int ttl);

// Default message handler
void p2p_message_default_handler(P2PMessage* msg);

// Free message
void p2p_message_free(P2PMessage* msg);

#endif
