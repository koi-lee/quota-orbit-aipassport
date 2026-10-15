#pragma once
#include <stdbool.h>
#include <stdint.h>
#ifdef ESP_PLATFORM
#include "lwip/sockets.h"
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif
/* Network byte order, matching esp_netif_ip_info_t and sockaddr_in. */
bool orbit_peer_on_subnet(const struct sockaddr_storage *peer, uint32_t address, uint32_t mask);
