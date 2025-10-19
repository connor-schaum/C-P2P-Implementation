#ifndef P2P_PEER_H
#define P2P_PEER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "DataStructures/Lists/LinkedList.h"

// Peer structure
typedef struct {
    char address[128];  // IP:PORT
    time_t last_seen;   // Last time we heard from this peer
} P2PPeer;

// Peer list structure
typedef struct {
    struct LinkedList peer_list;
} P2PPeerList;

// Create peer list
P2PPeerList* p2p_peer_list_create();

// Add peer to list (with duplicate checking)
int p2p_peer_list_add(P2PPeerList* list, const char* address, const char* node_id);

// Load peers from file into in-memory list
int p2p_peer_list_load_from_file(P2PPeerList* list, const char* node_id);

// Check if peer exists in file
int p2p_peer_exists_in_file(const char* address, const char* node_id);

// Remove peer from list
int p2p_peer_list_remove(P2PPeerList* list, const char* address);

// Find peer by address
P2PPeer* p2p_peer_list_find(P2PPeerList* list, const char* address);

// List all peers
void p2p_peer_list_print(P2PPeerList* list);

// Get peer count
int p2p_peer_list_count(P2PPeerList* list);

// Free peer list
void p2p_peer_list_free(P2PPeerList* list);

#endif
