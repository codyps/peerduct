#ifndef PEER_H_
#define PEER_H_

/* struct sockaddr_storage */
#include <netinet/in.h>

/* XXX: presently assumes that each 'peer' has a single method of reaching it.
 * this is inaccurate */

enum peer_type {
	PT_DIRECT,
	PT_SYM_NAT,       /* symetric */
	PT_FULL_CONE_NAT, /* full cone */
	PT_REST_CONE_NAT, /* restricted cone */
	PT_REST_PORT_NAT  /* restricted port */
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

struct peer {
	struct peer_addrinfo *pai;
	struct peer_id pid;
};

#endif
