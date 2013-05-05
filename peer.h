#ifndef PEER_H_
#define PEER_H_

#include <ccan/ccan/container_of.h>

/* struct sockaddr_storage */
#include <netinet/in.h>

/* XXX: presently assumes that each 'peer' has a single method of reaching it.
 * this is inaccurate */

enum peer_route_type {
	PRT_DIRECT,
	PRT_SYM_NAT,       /* symetric */
	PRT_FULL_CONE_NAT, /* full cone */
	PRT_REST_CONE_NAT, /* restricted cone */
	PRT_REST_PORT_NAT  /* restricted port */
};

struct peer_id {
	/* KAD id for now (128 bit) */
	uint8_t id[16];
};

/* XXX: this is a copy of addrinfo with route_type added. I don't like the
 * duplication, but it isn't clear how to avoid it. */
struct peer_addrinfo {
	enum peer_route_type	 pai_route_type;
	int			 pai_family;
	int			 pai_socktype;
	int			 pai_protocol;
	size_t			 pai_addrlen;
	struct sockaddr		*pai_addr;
	char			*pai_canonname;
	struct peer_addrinfo	*pai_next;
};

enum peer_type {
	PT_KAD,
	PT_BT_DHT
};

struct peer {
	enum peer_type type;
	struct peer   *next;
};

struct kad_peer {
	struct peer peer;

	uint8_t client_id[16];
	uint8_t  kad_udp_key[8];

	struct sockaddr_in udp;
	struct sockaddr_in tcp;

	uint8_t  verified; /* non-zero value indicates verified. */
	uint8_t  version;  /* 0 = kad v1, non-zero indicates v2 and is a
			      featured bitmask */
};

#define kad_peer_from_peer(p) container_of(p, struct kad_peer, peer)
void free_kad_peers(struct peer *peer);

#endif
