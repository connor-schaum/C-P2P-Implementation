#include "p2p_peer.h"
#include <unistd.h>

// Create peer list
P2PPeerList* p2p_peer_list_create() {
    P2PPeerList* list = malloc(sizeof(P2PPeerList));
    if (!list) return NULL;
    
    list->peer_list = linked_list_constructor();
    return list;
}

// Add peer to list (with duplicate checking)
int p2p_peer_list_add(P2PPeerList* list, const char* address, const char* node_id) {
    // Check if peer already exists in file (source of truth)
    if (p2p_peer_exists_in_file(address, node_id)) {
        printf("Peer %s already exists in file, skipping\n", address);
        return 0;  // Already exists
    }
    
    // Also check in-memory list for efficiency
    struct Node* current = list->peer_list.head;
    while (current != NULL) {
        P2PPeer* existing_peer = (P2PPeer*)current->data;
        if (strcmp(existing_peer->address, address) == 0) {
            return 0;  // Already exists in memory
        }
        current = current->next;
    }
    
    // Create new peer
    P2PPeer* new_peer = malloc(sizeof(P2PPeer));
    if (!new_peer) return -1;
    
    strncpy(new_peer->address, address, 127);
    new_peer->address[127] = '\0';
    new_peer->last_seen = time(NULL);
    
    // Add to peer list
    list->peer_list.insert(&list->peer_list, list->peer_list.length, new_peer, sizeof(P2PPeer));
    
    // Persist peer to file for bootstrapping
    char filename[64];
    snprintf(filename, sizeof(filename), "%s_PeerList.txt", node_id);
    
    FILE* peer_file = fopen(filename, "a");
    if (peer_file == NULL) {
        // If we can't open in append mode, try to create the file
        peer_file = fopen(filename, "w");
    }
    
    if (peer_file != NULL) {
        fprintf(peer_file, "%s\n", address);
        fclose(peer_file);
        printf("Persisted peer %s to file %s\n", address, filename);
    } else {
        printf("Error: Could not create or open %s file\n", filename);
    }
    
    printf("Added peer %s\n", address);
    return 1;  // Successfully added
}

// Load peers from file into in-memory list
int p2p_peer_list_load_from_file(P2PPeerList* list, const char* node_id) {
    char filename[64];
    snprintf(filename, sizeof(filename), "%s_PeerList.txt", node_id);
    
    FILE* peer_file = fopen(filename, "r");
    if (peer_file == NULL) {
        printf("No existing peer file found: %s\n", filename);
        return 0;  // No file to load from
    }
    
    char line[128];
    int loaded_count = 0;
    
    while (fgets(line, sizeof(line), peer_file) != NULL) {
        // Remove newline character
        line[strcspn(line, "\n")] = '\0';
        
        // Skip empty lines
        if (strlen(line) == 0) continue;
        
        // Add peer to in-memory list (without file persistence since it's already in file)
        P2PPeer* new_peer = malloc(sizeof(P2PPeer));
        if (!new_peer) continue;
        
        strncpy(new_peer->address, line, 127);
        new_peer->address[127] = '\0';
        new_peer->last_seen = time(NULL);
        
        // Add to peer list
        list->peer_list.insert(&list->peer_list, list->peer_list.length, new_peer, sizeof(P2PPeer));
        loaded_count++;
        printf("Loaded peer from file: %s\n", line);
    }
    
    fclose(peer_file);
    printf("Loaded %d peers from file %s\n", loaded_count, filename);
    return loaded_count;
}

// Check if peer exists in file
int p2p_peer_exists_in_file(const char* address, const char* node_id) {
    char filename[64];
    snprintf(filename, sizeof(filename), "%s_PeerList.txt", node_id);
    
    FILE* peer_file = fopen(filename, "r");
    if (peer_file == NULL) {
        return 0;  // File doesn't exist, so peer doesn't exist
    }
    
    char line[128];
    while (fgets(line, sizeof(line), peer_file) != NULL) {
        // Remove newline character
        line[strcspn(line, "\n")] = '\0';
        
        if (strcmp(line, address) == 0) {
            fclose(peer_file);
            return 1;  // Peer exists in file
        }
    }
    
    fclose(peer_file);
    return 0;  // Peer not found in file
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
