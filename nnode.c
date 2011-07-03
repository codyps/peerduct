#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include <unistd.h> /* getopt */
#include <ev.h>

#include "cfg_json.h"

struct node {
	char *host_str;
	char *port_str;


	ev_timer ping_watcher; /* a time out to indicate when to ping */
};

static void interact_cb(EV_P_ ev_io *w, int revents)
{
	/* At this point a ctrl fd is ready for reading */
	/* TODO: read from the associated FD for a command */
	/* TODO: process the recieved command */
}

static void ping_cb(EV_P_ ev_timer *w, int revents)
{
	/* TODO: send a PING rpc to the peer this event is associated with */
}

static int setup_listener(struct j_config *cfg)
{
	/* TODO: create a UDP socket */

	return -1;
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

	/* TODO: add PING timeout on node */
	ev_timer_init(&node->ping_watcher, ping_cb, 1., 1.); /* trigger repeatedly at 1 sec interval */
	ev_timer_start(loop, &node->ping_watcher);
	
	/* TODO: register node data structure in list of nodes */


	return -1;
}

static void listen_cb(EV_P_ ev_io *w, int revents)
{
	/* TODO: read the data and the data's source */

	/* TODO: if the node is already known process it based on our data of it */
		/* process_node(node, pkt, pkt_len); */

	/* TODO: otherwise, create a new node */
		/* new_node(loop, pkt, pkt_len); */

	/* TODO: accept the new connection */
	/* TODO: add a new rpc_watcher  for the node */
	/* TODO: add a new ping_watcher for the node */
}


static void do_event_loop(struct j_config *cfg)
{
	ev_io interact_watcher; /* interperates commands from a local source */
	ev_io listen_watcher;   /* listens for new connections from peers */
	ev_timer ping_watcher;  /* handles sending periodic pings to associated hosts */

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


	struct j_config cfg;
	int ret = j_config_load(&cfg, cfg_file);

	if (ret < 0)
		return ret;



	j_config_destroy(&cfg);

	return 0;
}
