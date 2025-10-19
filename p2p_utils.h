#ifndef P2P_UTILS_H
#define P2P_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Build peer list string from peer list
void p2p_build_peer_list_string(void* peer_list, char* peer_list_str, size_t str_size);

#endif
