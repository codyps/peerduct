#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include <unistd.h> /* getopt */

#include <sys/socket.h> /* udp */
#include <netinet/in.h> /* udp */

#include <ev.h>

#include "kademlia.h"
#include "cfg_json.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define container_of(ptr, type, member) ({ \
		const typeof( ((type *)0)->member ) *__mptr = (ptr); \
		(type *)( (char *)__mptr - offsetof(type,member) );})


#define VERSION_CURRENT 0x00

#define ENCODE_ID(id) \
	(id).bytes[ 0], (id).bytes[ 1], (id).bytes[ 2], (id).bytes[ 3], \
	(id).bytes[ 4], (id).bytes[ 5], (id).bytes[ 6], (id).bytes[ 7], \
	(id).bytes[ 8], (id).bytes[ 9], (id).bytes[10], (id).bytes[11], \
	(id).bytes[12], (id).bytes[13], (id).bytes[14], (id).bytes[15], \
	(id).bytes[16], (id).bytes[17], (id).bytes[18], (id).bytes[19]

#define PING_TOKEN 0x11
#define PONG_TOKEN 0x12

static void rand_kademlia_id(struct kademlia_id *kad)
{
	int i;
	for ( i = 0; i < ARRAY_SIZE(kad->dwords); i++) {
		kad->dwords[i] = rand();
	}
}

struct node {
	int sockfd; /**< shared between all udp stuff */

	char *host_str;
	char *port_str;

	ev_timer ping_watcher; /* a time out to indicate when to ping */


	struct kademlia_id id;

	struct sockaddr_storage saddr;
	socklen_t saddr_len;
};

static struct node _me, *me = &_me;

static void interact_cb(EV_P_ ev_io *w, int revents)
{
	/* At this point a ctrl fd is ready for reading */
	/* TODO: read from the associated FD for a command */
	/* TODO: process the recieved command */
}

static void setup_me(struct config *cfg)
{
	int ret = config_get_id(cfg, &me->id);

	if (ret < 0) {
		rand_kademlia_id(&me->id);
	}
}

static ssize_t send_pkt(struct node *node, enum pkt_token token, void *data, size_t data_len)
{
	/* TODO: don't ignore extra data */
	uint8_t buf [] = { VERSION_CURRENT, ENCODE_ID(me->id), token };
	return sendto(nd->sockfd, buf, sizeof(buf), 0,
			(struct sockaddr *)&nd->saddr, nd->saddr_len);
}

static void ping_cb(EV_P_ ev_timer *w, int revents)
{
	struct node *nd = container_of(w, typeof(*nd), ping_watcher);

	/* TODO: send a PING rpc to the peer this event is associated with */
	/* TODO: update possitioning based on how it has responded to past pings */


	ssize_t ret = send_pkt(nd, PING_TOKEN, NULL, 0);

	if (ret < sizeof(buf)) {
		/* TODO: handle */
	}
}

static int setup_listener(struct config *cfg)
{
	/* create a UDP socket */
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		return sock;


	/* FIXME: if the cfg specifies a port, use it */

	return sock;
}


/* called when a ping has been recieved, pkt contains the remaining portion of
 * the packet following the PING token */
static void handle_ping(struct node *node, void *pkt, size_t pkt_len)
{
	printf("GOT PING, sending pong.\n");

	ssize_t ret = send_pkt(node, PONG_TOKEN, pkt, pkt_len);
	if (ret < 0) {
		fprintf(stderr, "PONG send failure.");
	}
}

static void process_node(struct node *node, void *pkt, size_t pkt_len)
{
	/* TODO: check that pkt is valid */
	/* TODO: is node fully authorized? if not, see if pkt continues auth */

	/* TODO: from contents of packet, determine the need for responce.
	 * Possibly reset the timers associated with the node */
}

static int new_node(struct ev_loop *loop, void *pkt, size_t pkt_len)
{
	/* TODO: see if pkt is a valid first packet */
	/* TODO: create node data structure */
	struct node *node = malloc(sizeof(*node));
	if (!node)
		return -1;

	/* add PING timeout on node */
	ev_timer_init(&node->ping_watcher, ping_cb, 1., 1.); /* trigger repeatedly at 1 sec interval */
	ev_timer_start(loop, &node->ping_watcher);
	
	/* TODO: register node data structure in list of nodes */

	return -1;
}

/* handles incomming packets on a UDP socket */
static void listen_cb(EV_P_ ev_io *w, int revents)
{
	/* TODO: read the data and the data's source */

	/* TODO: if the node is already known process it based on our data of it */
		/* process_node(node, pkt, pkt_len); */

	/* TODO: otherwise, create a new node */
		/* new_node(loop, pkt, pkt_len); */

}


static void do_event_loop(struct config *cfg)
{
	ev_io interact_watcher; /* interperates commands from a local source */
	ev_io listen_watcher;   /* listens for new connections from peers */

	struct ev_loop *loop = EV_DEFAULT;
	
	ev_io_init(&interact_watcher, interact_cb, STDIN_FILENO, EV_READ);
	ev_io_start(loop, &interact_watcher);

	int sock = setup_listener(cfg);
	ev_io_init(&listen_watcher, listen_cb, sock, EV_READ);
	ev_io_start(loop, &listen_watcher);

	ev_run(loop, 0);
}


#define PROG_NAME(argc, argv) (argc?argv[0]:"nnode")
void usage(char *prog)
{
	printf(
"\
usage: %s [options]\n\
options:\n\
	-c<config file>		specify a config file\n\
"
	, prog);

}

int main(int argc, char **argv)
{
	const char optstring [] = ":hc:";
	char *cfg_file = NULL;
	int c, errflg = 0;

	while ((c = getopt(argc, argv, optstring)) != -1) {
		switch(c) {
		case 'h':
			usage(PROG_NAME(argc, argv));
			return -1;
			break;
		case 'c':
			cfg_file = optarg;
			break;
		case ':':
			fprintf(stderr, "option -%c requires an operand\n",
					optopt);
			errflg++;
			break;
		case '?':
			fprintf(stderr, "unrecognized option -%c\n",
					optopt);
			errflg++;
			break;
		default:
			fprintf(stderr, "what\n");
			errflg++;
			break;
		}
	}

	if (!cfg_file) {
		fprintf(stderr, "no config file specified.\n");
		errflg++;
	}

	if (errflg) {
		usage(PROG_NAME(argc, argv));
		return -1;
	}


	struct config cfg;
	int ret = config_load(&cfg, cfg_file);

	if (ret < 0)
		return ret;

	setup_me(&cfg);

	do_event_loop(&cfg);

	config_destroy(&cfg);

	return 0;
}
