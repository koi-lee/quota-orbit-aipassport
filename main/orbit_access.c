#include "orbit_access.h"
#include <string.h>

bool orbit_peer_on_subnet(const struct sockaddr_storage *peer, uint32_t address, uint32_t mask) {
    uint32_t ipv4;
    if (!peer || !address || !mask)
        return false;
    if (peer->ss_family == AF_INET) {
        ipv4 = ((const struct sockaddr_in *)peer)->sin_addr.s_addr;
    } else if (peer->ss_family == AF_INET6) {
        /* ESP-IDF's dual-stack HTTP listener reports IPv4 clients as ::ffff:a.b.c.d. */
        const unsigned char *bytes =
            (const unsigned char *)&((const struct sockaddr_in6 *)peer)->sin6_addr;
        static const unsigned char prefix[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255};
        if (memcmp(bytes, prefix, sizeof(prefix)))
            return false;
        memcpy(&ipv4, bytes + sizeof(prefix), sizeof(ipv4));
    } else {
        return false;
    }
    return ipv4 != 0 && (ipv4 & mask) == (address & mask);
}
