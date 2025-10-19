#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "p2p_message.h"
#include "p2p_peer.h"
#include "p2p_network.h"

int main(int argc, char* argv[]) {
    char* node_address = "127.0.0.1:1248";  // Default
    char* connect_to = NULL;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--address") == 0 && i + 1 < argc) {
            node_address = argv[++i];
        }
        else if (strcmp(argv[i], "--connect-to") == 0 && i + 1 < argc) {
            connect_to = argv[++i];
        }
    }
    
    printf("Starting P2P node on %s\n", node_address);
    
    // Extract port from address
    char* colon = strrchr(node_address, ':');
    if (!colon) {
        printf("Error: Address must be in format IP:PORT\n");
        return 1;
    }
    int port = atoi(colon + 1);
    
    // Create network
    P2PNetwork* network = p2p_network_create(port, node_address, p2p_message_default_handler);
    if (!network) {
        printf("Failed to create P2P network\n");
        return 1;
    }
    
    // Start network
    if (p2p_network_start(network) != 0) {
        printf("Failed to start P2P network\n");
        p2p_network_free(network);
        return 1;
    }
    
    // Connect if specified
    if (connect_to) {
        p2p_network_connect(network, connect_to);
    }
    
    printf("P2P Node ready.\n");
    printf("Commands: 'send <address> <type> <data>', 'broadcast <type> <data>', 'list', 'quit'\n");
    
    // Command loop
    char command[256];
    while (1) {
        printf("> ");
        if (!fgets(command, sizeof(command), stdin)) break;
        
        command[strcspn(command, "\n")] = 0;
        
        if (strcmp(command, "quit") == 0) {
            break;
        }
        else if (strcmp(command, "list") == 0) {
            p2p_peer_list_print(network->peer_list);
        }
        else if (strncmp(command, "send ", 5) == 0) {
            char* args = command + 5;
            char* address = strtok(args, " ");
            char* type = strtok(NULL, " ");
            char* data = strtok(NULL, "");
            
            if (address && type && data) {
                p2p_network_send(network, address, type, data);
            } else {
                printf("Usage: send <address> <type> <data>\n");
            }
        }
        else if (strncmp(command, "broadcast ", 10) == 0) {
            char* args = command + 10;
            char* type = strtok(args, " ");
            char* data = strtok(NULL, "");
            
            if (type && data) {
                p2p_network_broadcast(network, type, data);
            } else {
                printf("Usage: broadcast <type> <data>\n");
            }
        }
        else {
            printf("Unknown command. Try 'send <address> <type> <data>', 'broadcast <type> <data>', 'list', or 'quit'\n");
        }
    }
    
    p2p_network_free(network);
    return 0;
}
