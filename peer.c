#include "peer.h"
#include <stdlib.h>

#include "useful.h"

void free_kad_peers(struct peer *peer)
{
	while(peer) {
		struct peer *tmp = peer->next;
		free(container_of(peer, struct kad_peer, peer));
		peer = tmp;
	}
}
