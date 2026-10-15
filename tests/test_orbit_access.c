#include "orbit_access.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    struct sockaddr_storage peer = {0};
    struct sockaddr_in *v4 = (struct sockaddr_in *)&peer;
    uint32_t ip = inet_addr("192.168.4.1"), mask = inet_addr("255.255.255.0");
    v4->sin_family = AF_INET;
    v4->sin_addr.s_addr = inet_addr("192.168.4.2");
    assert(orbit_peer_on_subnet(&peer, ip, mask));
    v4->sin_addr.s_addr = inet_addr("192.168.1.2");
    assert(!orbit_peer_on_subnet(&peer, ip, mask));
    memset(&peer, 0, sizeof(peer));
    struct sockaddr_in6 *v6 = (struct sockaddr_in6 *)&peer;
    v6->sin6_family = AF_INET6;
    assert(inet_pton(AF_INET6, "::ffff:192.168.4.2", &v6->sin6_addr) == 1);
    assert(peer.ss_family != AF_INET); /* The previous gate rejected this valid client. */
    assert(orbit_peer_on_subnet(&peer, ip, mask));
    assert(inet_pton(AF_INET6, "::ffff:192.168.1.2", &v6->sin6_addr) == 1);
    assert(!orbit_peer_on_subnet(&peer, ip, mask));
    assert(inet_pton(AF_INET6, "2001:db8::c0a8:402", &v6->sin6_addr) == 1);
    assert(!orbit_peer_on_subnet(&peer, ip, mask));
    assert(inet_pton(AF_INET6, "::192.168.4.2", &v6->sin6_addr) == 1);
    assert(!orbit_peer_on_subnet(&peer, ip, mask));
    assert(!orbit_peer_on_subnet(NULL, ip, mask));
    assert(!orbit_peer_on_subnet(&peer, ip, 0));
    peer.ss_family = AF_UNSPEC;
    assert(!orbit_peer_on_subnet(&peer, ip, mask));
    puts("Provisioning IPv4 / mapped IPv6 subnet gate: PASS");
    return 0;
}
