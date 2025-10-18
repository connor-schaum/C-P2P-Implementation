#include "p2p_network.h"

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
        
        char* client_ip = inet_ntoa(client_addr.sin_addr);
        
        // Handle connection
        P2PMessage msg;
        if (read(client_socket, &msg, sizeof(P2PMessage)) == sizeof(P2PMessage)) {
            // Add sender to peer list (bidirectional peer discovery)
            char sender_address[128];
            snprintf(sender_address, sizeof(sender_address), "%s:1249", client_ip); // Default port for now
            p2p_peer_list_add(network->peer_list, sender_address, msg.sender);
            
            if (network->message_handler) {
                network->message_handler(&msg);
            }
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
    
    *colon = '\0';
    char* ip = (char*)address;
    int port = atoi(colon + 1);
    
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
    msg.ttl = 5;
    
    write(client_socket, &msg, sizeof(P2PMessage));
    close(client_socket);
    
    printf("Sent %s to %s\n", type, address);
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
    p2p_peer_list_add(network->peer_list, address, "target_peer");
    
    // Send discovery message
    return p2p_network_send(network, address, "DISCOVERY", "Hello from P2P node!");
}

// Stop network
void p2p_network_stop(P2PNetwork* network) {
    // Implementation for graceful shutdown
}

// Free network
void p2p_network_free(P2PNetwork* network) {
    if (network->peer_list) {
        p2p_peer_list_free(network->peer_list);
    }
    free(network);
}
