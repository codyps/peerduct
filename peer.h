#ifndef PEER_H_
#define PEER_H_

/* XXX: presently assumes that each 'peer' has a single method of reaching it.
 * this is inaccurate */

enum peer_type {
	PT_DIRECT,
	PT_SYM_NAT,       /* symetric */
	PT_FULL_CONE_NAT, /* full cone */
	PT_REST_CONE_NAT, /* restricted cone */
	PT_REST_PORT_NAT  /* restricted port */
};

struct peer {
	enum peer_type pt;

	struct sockaddr_storage saddr;
	int saddr_len;


};

#endif
