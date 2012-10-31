#include <stdio.h>


#include <sys/types.h>	/* socket, getaddrinfo */
#include <sys/socket.h>	/* socket, getaddrinfo */

#include <netdb.h>	/* getaddrinfo */
#include <netinet/in.h>	/* udp */

#include <stdarg.h>	/* va_* */
#include <unistd.h>	/* getopt */


static int warn(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int x = vfprintf(stderr, fmt, ap);
	va_end(ap);
	return x;
}

static void print_kad_peers(struct peer *peer)
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

static int load_peers(char *path)
{

	FILE *in = fopen(path, "rb");
	if (!in) {
		return 4;
	}

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
}

struct ctrl_args {
	
};

static void *ctrl_thread(void *ca_v)
{
	struct ctrl_args *ca = ca_v;



	return NULL;
}

void spawn_ctrl_thread(struct locked_list *ctrl_thread_pool)
{
	pthread_t pth;
	struct ctrl_args 
	int x = pthread_create(&pth, NULL, ctrl_thread);
}

static void usage(char *pname)
{
	warn(
		"usage: %s	[options]\n"
		"	-h		show help\n"
		"	-S <path>	location to place control socket\n"
	    , pname);

}

static const char optstr[] = ":p:f:l:";
int main(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, optstr)) != -1) {
		switch(c) {
		case 'S':
			warn("spawning ctrl listener\n");
			spwan_ctrl_thread();
			break;
		case ':':
			warn("option '-%c' requires an argument\n", optopt);
			break;
		case '?':
			warn("option '-%c' is unrecognized\n", optopt);
			break;
		default:
			warn("opt: %c\n", c);
			return -1;
		}
	}

	return 0;
}
