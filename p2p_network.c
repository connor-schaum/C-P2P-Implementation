#include "p2p_network.h"
#include "p2p_utils.h"

// Global network reference for callbacks
static P2PNetwork* g_network = NULL;

// Server thread function
void* p2p_server_thread(void* arg) {
    P2PNetwork* network = (P2PNetwork*)arg;
    
    // Create server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        printf("Failed to create server socket\n");
        return NULL;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind to port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(network->port);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Failed to bind to port %d\n", network->port);
        close(server_socket);
        return NULL;
    }
    
    // Listen
    if (listen(server_socket, 10) < 0) {
        printf("Failed to listen\n");
        close(server_socket);
        return NULL;
    }
    
    printf("Server running on port %d\n", network->port);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        
        if (client_socket < 0) continue;
        
        // Handle connection - read message type first
        char msg_type[32];
        int type_bytes = read(client_socket, msg_type, 32);
        if (type_bytes == 32) {
            msg_type[31] = '\0';  // Ensure null termination
            printf("DEBUG: Received message type: '%s'\n", msg_type);
            
            if (strcmp(msg_type, "DISCOVERY") == 0) {
                printf("DEBUG: Handling as discovery message\n");
                // Read the rest of the discovery message
                char disc_data[sizeof(DiscoveryMessage) - 32];
                int disc_bytes = read(client_socket, disc_data, sizeof(DiscoveryMessage) - 32);
                if (disc_bytes == sizeof(DiscoveryMessage) - 32) {
                    // Reconstruct the full discovery message
                    DiscoveryMessage disc_msg;
                    memcpy(&disc_msg, msg_type, 32);
                    memcpy(((char*)&disc_msg) + 32, disc_data, sizeof(DiscoveryMessage) - 32);
                    
                    printf("DEBUG: Received DISCOVERY message from %s\n", disc_msg.sender);
                    // Add sender to peer list using the sender's address from the message
                    p2p_peer_list_add(network->peer_list, disc_msg.sender, network->node_id);
                    
                    // Handle discovery message
                    if (disc_msg.ttl > 0) {
                        // Parse and add peers from the received peer list
                        char* peer_list_copy = strdup(disc_msg.peer_list);
                        char* token = strtok(peer_list_copy, ",");
                        while (token != NULL) {
                            // Skip empty tokens and self
                            if (strlen(token) > 0 && strcmp(token, network->node_id) != 0) {
                                int added = p2p_peer_list_add(network->peer_list, token, network->node_id);
                                // If peer was newly added, automatically connect to it
                                if (added > 0) {
                                    printf("Auto-connecting to newly discovered peer: %s\n", token);
                                    p2p_network_connect(network, token);
                                }
                            }
                            token = strtok(NULL, ",");
                        }
                        free(peer_list_copy);
                        
                        // Send back our peer list with decremented TTL
                        char our_peer_list[1024];
                        p2p_build_peer_list_string(network->peer_list, our_peer_list, sizeof(our_peer_list));
                        
                        DiscoveryMessage response;
                        strncpy(response.type, "DISCOVERY", 31);
                        response.type[31] = '\0';
                        strncpy(response.sender, network->node_id, 63);
                        response.sender[63] = '\0';
                        response.ttl = disc_msg.ttl - 1;
                        strncpy(response.peer_list, our_peer_list, 1023);
                        response.peer_list[1023] = '\0';
                        
                        write(client_socket, &response, sizeof(DiscoveryMessage));
                        
                        // Forward discovery to all other peers (propagation)
                        if (disc_msg.ttl > 1) {
                            printf("Forwarding discovery with TTL=%d to other peers\n", disc_msg.ttl - 1);
                            struct Node* current = network->peer_list->peer_list.head;
                            while (current != NULL) {
                                P2PPeer* peer = (P2PPeer*)current->data;
                                // Don't send back to the original sender
                                if (strcmp(peer->address, disc_msg.sender) != 0) {
                                    printf("Forwarding to peer: %s\n", peer->address);
                                    p2p_network_send_discovery(network, peer->address, disc_msg.ttl - 1, our_peer_list);
                                }
                                current = current->next;
                            }
                        }
                        
                        // Also try to connect to peers from the original discovery
                        char* original_peer_list = strdup(disc_msg.peer_list);
                        char* discovery_token = strtok(original_peer_list, ",");
                        while (discovery_token != NULL) {
                            if (strlen(discovery_token) > 0 && strcmp(discovery_token, network->node_id) != 0) {
                                // Check if we already know this peer
                                int exists = 0;
                                struct Node* check = network->peer_list->peer_list.head;
                                while (check != NULL) {
                                    P2PPeer* existing_peer = (P2PPeer*)check->data;
                                    if (strcmp(existing_peer->address, discovery_token) == 0) {
                                        exists = 1;
                                        break;
                                    }
                                    check = check->next;
                                }
                                
                                if (!exists) {
                                    printf("Auto-connecting to peer from discovery: %s\n", discovery_token);
                                    p2p_network_connect(network, discovery_token);
                                }
                            }
                            discovery_token = strtok(NULL, ",");
                        }
                        free(original_peer_list);
                    }
                }
            } else {
                printf("DEBUG: Handling as regular message\n");
                // Read the rest of the regular message
                char msg_data[sizeof(P2PMessage) - 32];
                int msg_bytes = read(client_socket, msg_data, sizeof(P2PMessage) - 32);
                if (msg_bytes == sizeof(P2PMessage) - 32) {
                    // Reconstruct the full regular message
                    P2PMessage msg;
                    memcpy(&msg, msg_type, 32);
                    memcpy(((char*)&msg) + 32, msg_data, sizeof(P2PMessage) - 32);
                    
                    if (network->message_handler) {
                        network->message_handler(&msg);
                    }
                } else {
                    printf("DEBUG: Failed to read regular message (got %d bytes, expected %zu)\n", 
                           msg_bytes, sizeof(P2PMessage) - 32);
                }
            }
        } else {
            printf("DEBUG: Failed to read message type (got %d bytes, expected 32)\n", type_bytes);
        }
        
        close(client_socket);
    }
    
    return NULL;
}

// Create network
P2PNetwork* p2p_network_create(int port, const char* node_id, message_handler_t handler) {
    P2PNetwork* network = malloc(sizeof(P2PNetwork));
    if (!network) return NULL;
    
    network->port = port;
    strncpy(network->node_id, node_id, 63);
    network->node_id[63] = '\0';
    network->peer_list = p2p_peer_list_create();
    network->message_handler = handler;
    
    // Load existing peers from file
    p2p_peer_list_load_from_file(network->peer_list, node_id);
    
    // Bootstrap: automatically connect to loaded peers
    struct Node* current = network->peer_list->peer_list.head;
    while (current != NULL) {
        P2PPeer* peer = (P2PPeer*)current->data;
        printf("Bootstrap: connecting to peer %s\n", peer->address);
        p2p_network_connect(network, peer->address);
        current = current->next;
    }
    
    g_network = network;
    return network;
}

// Start network (starts server thread)
int p2p_network_start(P2PNetwork* network) {
    pthread_t server_tid;
    if (pthread_create(&server_tid, NULL, p2p_server_thread, network) != 0) {
        return -1;
    }
    pthread_detach(server_tid);
    return 0;
}

// Send message to specific address
int p2p_network_send(P2PNetwork* network, const char* address, const char* type, const char* data) {
    // Parse address
    char* colon = strrchr(address, ':');
    if (!colon) return -1;
    
    // Make a copy to avoid modifying the original
    char address_copy[128];
    strncpy(address_copy, address, sizeof(address_copy) - 1);
    address_copy[sizeof(address_copy) - 1] = '\0';
    
    char* colon_copy = strrchr(address_copy, ':');
    *colon_copy = '\0';
    char* ip = address_copy;
    int port = atoi(colon_copy + 1);
    
    // Create client socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) return -1;
    
    // Connect
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(client_socket);
        return -1;
    }
    
    // Send message
    P2PMessage msg;
    strncpy(msg.type, type, 31);
    msg.type[31] = '\0';
    strncpy(msg.sender, network->node_id, 63);
    msg.sender[63] = '\0';
    strncpy(msg.data, data, 255);
    msg.data[255] = '\0';
    
    write(client_socket, &msg, sizeof(P2PMessage));
    close(client_socket);
    
    printf("Sent %s to %s\n", type, address);
    return 0;
}

// Send discovery message
int p2p_network_send_discovery(P2PNetwork* network, const char* address, int ttl, const char* peer_list) {
    // Parse address
    char* colon = strrchr(address, ':');
    if (!colon) return -1;
    
    // Make a copy to avoid modifying the original
    char address_copy[128];
    strncpy(address_copy, address, sizeof(address_copy) - 1);
    address_copy[sizeof(address_copy) - 1] = '\0';
    
    char* colon_copy = strrchr(address_copy, ':');
    *colon_copy = '\0';
    char* ip = address_copy;
    int port = atoi(colon_copy + 1);
    
    // Create client socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) return -1;
    
    // Connect
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(client_socket);
        return -1;
    }
    
    // Send discovery message
    DiscoveryMessage msg;
    strncpy(msg.type, "DISCOVERY", 31);
    msg.type[31] = '\0';
    strncpy(msg.sender, network->node_id, 63);
    msg.sender[63] = '\0';
    msg.ttl = ttl;
    strncpy(msg.peer_list, peer_list, 1023);
    msg.peer_list[1023] = '\0';
    
    write(client_socket, &msg, sizeof(DiscoveryMessage));
    close(client_socket);
    
    printf("Sent DISCOVERY to %s with TTL=%d\n", address, ttl);
    return 0;
}

// Broadcast message to all peers
int p2p_network_broadcast(P2PNetwork* network, const char* type, const char* data) {
    struct Node* current = network->peer_list->peer_list.head;
    int sent_count = 0;
    
    while (current != NULL) {
        P2PPeer* peer = (P2PPeer*)current->data;
        if (p2p_network_send(network, peer->address, type, data) == 0) {
            sent_count++;
        }
        current = current->next;
    }
    
    printf("Broadcast %s to %d peers\n", type, sent_count);
    return sent_count;
}

// Connect to a peer
int p2p_network_connect(P2PNetwork* network, const char* address) {
    printf("Connecting to: %s\n", address);
    
    // Add target peer to our peer list
    p2p_peer_list_add(network->peer_list, address, network->node_id);
    
    // Build our peer list string
    char peer_list_str[1024];
    p2p_build_peer_list_string(network->peer_list, peer_list_str, sizeof(peer_list_str));
    
    // Send discovery message to all peers
    struct Node* current = network->peer_list->peer_list.head;
    int sent_count = 0;
    
    while (current != NULL) {
        P2PPeer* peer = (P2PPeer*)current->data;
        printf("Sending discovery to peer: %s\n", peer->address);
        if (p2p_network_send_discovery(network, peer->address, 3, peer_list_str) == 0) {
            sent_count++;
        }
        current = current->next;
    }
    
    printf("Sent discovery to %d peers\n", sent_count);
    return sent_count;
}

// Stop network
void p2p_network_stop(P2PNetwork* network) {
    (void)network;
    // Implementation for graceful shutdown
}

// Free network
void p2p_network_free(P2PNetwork* network) {
    if (network->peer_list) {
        p2p_peer_list_free(network->peer_list);
    }
    free(network);
}
