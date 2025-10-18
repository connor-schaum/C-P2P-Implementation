#include "p2p_peer.h"

// Create peer list
P2PPeerList* p2p_peer_list_create() {
    P2PPeerList* list = malloc(sizeof(P2PPeerList));
    if (!list) return NULL;
    
    list->peer_list = linked_list_constructor();
    return list;
}

// Add peer to list (with duplicate checking)
int p2p_peer_list_add(P2PPeerList* list, const char* address, const char* peer_id) {
    // Check if peer already exists
    struct Node* current = list->peer_list.head;
    while (current != NULL) {
        P2PPeer* existing_peer = (P2PPeer*)current->data;
        if (strcmp(existing_peer->address, address) == 0) {
            return 0;  // Already exists
        }
        current = current->next;
    }
    
    // Create new peer
    P2PPeer* new_peer = malloc(sizeof(P2PPeer));
    if (!new_peer) return -1;
    
    strncpy(new_peer->address, address, 127);
    new_peer->address[127] = '\0';
    strncpy(new_peer->id, peer_id, 63);
    new_peer->id[63] = '\0';
    new_peer->last_seen = time(NULL);
    
    // Add to peer list
    list->peer_list.insert(&list->peer_list, list->peer_list.length, new_peer, sizeof(P2PPeer));
    
    printf("Added peer %s\n", address);
    return 1;  // Successfully added
}

// Remove peer from list
int p2p_peer_list_remove(P2PPeerList* list, const char* address) {
    struct Node* current = list->peer_list.head;
    int index = 0;
    
    while (current != NULL) {
        P2PPeer* peer = (P2PPeer*)current->data;
        if (strcmp(peer->address, address) == 0) {
            list->peer_list.remove(&list->peer_list, index);
            free(peer);
            return 1;  // Successfully removed
        }
        current = current->next;
        index++;
    }
    
    return 0;  // Not found
}

// Find peer by address
P2PPeer* p2p_peer_list_find(P2PPeerList* list, const char* address) {
    struct Node* current = list->peer_list.head;
    
    while (current != NULL) {
        P2PPeer* peer = (P2PPeer*)current->data;
        if (strcmp(peer->address, address) == 0) {
            return peer;
        }
        current = current->next;
    }
    
    return NULL;  // Not found
}

// List all peers
void p2p_peer_list_print(P2PPeerList* list) {
    printf("Known peers (%d):\n", list->peer_list.length);
    struct Node* current = list->peer_list.head;
    int index = 0;
    while (current != NULL) {
        P2PPeer* peer = (P2PPeer*)current->data;
        printf("  %d: %s\n", index, peer->address);
        current = current->next;
        index++;
    }
}

// Get peer count
int p2p_peer_list_count(P2PPeerList* list) {
    return list->peer_list.length;
}

// Free peer list
void p2p_peer_list_free(P2PPeerList* list) {
    // Free all peers
    struct Node* current = list->peer_list.head;
    while (current != NULL) {
        P2PPeer* peer = (P2PPeer*)current->data;
        free(peer);
        current = current->next;
    }
    
    free(list);
}
