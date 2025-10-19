#include "p2p_utils.h"
#include "p2p_peer.h"

// Build peer list string from peer list
void p2p_build_peer_list_string(void* peer_list, char* peer_list_str, size_t str_size) {
    peer_list_str[0] = '\0';  // Start with empty string
    
    P2PPeerList* pl = (P2PPeerList*)peer_list;
    struct Node* current = pl->peer_list.head;
    int first = 1;
    
    while (current != NULL) {
        P2PPeer* peer = (P2PPeer*)current->data;
        
        if (!first) {
            strncat(peer_list_str, ",", str_size - strlen(peer_list_str) - 1);
        }
        
        strncat(peer_list_str, peer->address, str_size - strlen(peer_list_str) - 1);
        first = 0;
        current = current->next;
    }
}
