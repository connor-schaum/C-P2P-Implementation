#include "p2p_message.h"

// Create a new message
P2PMessage* p2p_message_create(const char* type, const char* sender, const char* data) {
    P2PMessage* msg = malloc(sizeof(P2PMessage));
    if (!msg) return NULL;
    
    strncpy(msg->type, type, 31);
    msg->type[31] = '\0';
    strncpy(msg->sender, sender, 63);
    msg->sender[63] = '\0';
    strncpy(msg->data, data, 255);
    msg->data[255] = '\0';
    
    return msg;
}

// Default message handler
void p2p_message_default_handler(P2PMessage* msg) {
    printf("Received %s from %s: %s\n", msg->type, msg->sender, msg->data);
}

// Free message
void p2p_message_free(P2PMessage* msg) {
    free(msg);
}
