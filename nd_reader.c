#include <stdio.h>

#include "nodes_dat.h"

#include <sys/socket.h>	/* getnameinfo */
#include <netdb.h>	/* getnameinfo */

void print_kad_peers(struct peer *peer)
{
	char host[1024];
	char serv[1024];
	while(peer) {
		struct kad_peer *kp = kad_peer_from_peer(peer);

		getnameinfo((struct sockaddr *)&kp->tcp, sizeof(kp->tcp),
				host, sizeof(host),
				serv, sizeof(serv),
				NI_NUMERICHOST | NI_NUMERICSERV);

		printf("%15s %s\n", host, serv);

		peer = peer->next;
	}
}

int main(int argc, char **argv)
{

#if 0
	if (argc > 1) {
		fprintf(stderr, "usage: %s < nodes.dat\n", argv[0]);
		return 1;
	}
#endif

	FILE *in = fopen(argv[1], "rb");

	struct nd_parse_ctx npc;
	if (nd_parse_init(&npc))
		return 2;

	char buf[2048];
	size_t r;
	while((r = fread(buf, 1, sizeof(buf), in)) > 0) {
		int p = nd_parse_proc(&npc, buf, r);
		if (p < 0) {
			nd_parse_destroy(&npc);
			printf("god: %d\n", p);
			return 3;
		}
	}

	
	struct peer *peers = nd_parse_get_peers(&npc);
	if (!peers) {
		nd_parse_destroy(&npc);
		return 4;
	}


	print_kad_peers(peers);
	free_kad_peers(peers);

	nd_parse_destroy(&npc);

	return 0;
}
