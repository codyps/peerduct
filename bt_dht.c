/* Another kademlia variant */
#include <stdio.h>

#include <ccan/darray/darray.h>
#include <ccan/err/err.h>
#include <ccan/net/net.h>
#include <ccan/list/list.h>
#include <rbtree/rbtree.h>
#include <penny/print.h>

#include <ev.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/* XXX: because bucket splitting is deterministic, there should be a way
 * to track it without start & end. */
struct bucket {
	struct rb_node node;
	int num_nodes;
	int time_last_changed;
};

struct routing_table {
	size_t bucket_ct;
	size_t nodes_per_bucket; /* K */
	struct rb_root buckets;
};

typedef struct bt_node_id {
	char id[160/8]; /* 20 */
} bt_node_id;

enum KRPC_ERROR {
	KE_GENERIC = 201,
	KE_SERVER = 202,
	KE_PROTOCOL = 203,
	KE_UNK_METHOD = 204
};

static void dict_begin(darray_char *d)
{
	darray_append(*d, 'd');
}

static void dict_end(darray_char *d)
{
	darray_append(*d, 'e');
}

static void encode_bytes(darray_char *d, const char *bytes, size_t len)
{
	darray_printf(*d, "%zu:%*s", len, len, bytes);
}

static void encode_string(darray_char *d, const char *str)
{
	encode_bytes(d, str, strlen(str));
}

static void encode_id(darray_char *d, bt_node_id *id)
{
	encode_bytes(d, id->id, sizeof(id->id));
}

static void encode_ping(darray_char *d, bt_node_id *id)
{
	dict_begin(d);

	/* a = { "id" : MY_NODE_ID } */
	encode_string(d, "a");
	dict_begin(d);
	encode_string(d, "id");
	encode_id(d, id);
	dict_end(d);

	/* q = "ping" */
	encode_string(d, "q");
	encode_string(d, "ping");

	/* t = SOME_ID */
	encode_string(d, "t");
	encode_string(d, "tt"); /* some ID */

	/* y = "q" */
	encode_string(d, "y");
	encode_string(d, "q");

	dict_end(d);
}

static void encode_ping_response(darray_char *d, bt_node_id *my_id)
{
	dict_begin(d);

	/* r */
	encode_string(d, "r");
	dict_begin(d);
	encode_string(d, "id");
	encode_id(d, my_id);
	dict_end(d);

	/* t = SOME_ID */
	encode_string(d, "t");
	encode_string(d, "tt"); /* some id */

	/* y */
	encode_string(d, "y");
	encode_string(d, "r");

	dict_end(d);
}

static void load_random_id(bt_node_id *id)
{
	FILE *f = fopen("/dev/urandom", "r");
	if (!f)
		err(1, "could not open random file");

	size_t s = fread(id->id, 1, sizeof(id->id), f);
	if (s != sizeof(id->id))
		err(1, "could not read random file");

	fclose(f);
}

struct sock;
struct peer {
	struct sock *parent;
	struct list_node node;

	const char *name;
	const char *service;
	struct addrinfo *addr;

	ev_timer  stale_timer;
	ev_tstamp last_activity;

	/* data */
	bt_node_id id;
};

struct sock {
	ev_io w;
	struct list_head peers;
	darray_char buf;

	bt_node_id my_id;
};

#define PEER_TIMEOUT 10.

static void send_ping_to_peer(struct peer *p)
{
	struct sock *s = p->parent;

	darray_reset(s->buf);
	encode_ping(&s->buf, &s->my_id);

	printf("pinging %s:%s ::", p->name, p->service);
	print_bytes_as_cstring(s->buf.item, darray_size(s->buf), stdout);
	putchar('\n');

	ssize_t r = sendto(s->w.fd, s->buf.item, darray_size(s->buf),
			0, p->addr->ai_addr, p->addr->ai_addrlen);

	if (r == -1)
		warnx("sendto failed");
}

static void on_peer_timeout(EV_P_ ev_timer *w, int revents)
{
	struct peer *p = container_of(w, typeof(*p), stale_timer);
	ev_tstamp after = p->last_activity - ev_now(EV_A) + PEER_TIMEOUT;
	if (after < 0.) {
		send_ping_to_peer(p);

		ev_timer_set(w, PEER_TIMEOUT, 0.);
		ev_timer_start(EV_A_ w);
	} else {
		ev_timer_set(w, after, 0.);
		ev_timer_start(EV_A_ w);
	}
}

static void sock_cb(EV_P_ ev_io *w, int revents)
{
	char buf[2048];
	struct sock *s = container_of(w, typeof(*s), w);
	struct sockaddr_storage addr;
	socklen_t slen = sizeof(s);

	ssize_t l = recvfrom(w->fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &slen);

	if (l == -1) {
		warnx("recvfrom");
	}

	printf("recv'd packet of size %zd, sockaddr size \n", l);
	print_bytes_as_cstring(buf, l, stdout);
	putchar('\n');
}

static void resolve_peer(struct peer *p)
{
	struct addrinfo *addr = net_client_lookup(p->name, p->service,
					AF_INET, SOCK_DGRAM);

	if (!addr)
		errx(1, "could not resolve peer.");

	if (p->addr)
		freeaddrinfo(p->addr);
	p->addr = addr;
}

static struct peer *add_peer(EV_P_ struct sock *s, char *name, char *service)
{
	struct peer *p = malloc(sizeof(*p));
	if (!p) {
		warnx("peer allocation failed.");
		return NULL;
	}

	*p = (typeof(*p)) {
		.parent = s,
		.name = name,
		.service = service,
	};

	resolve_peer(p);

	ev_init(&p->stale_timer, on_peer_timeout);
	ev_timer_set(&p->stale_timer, PEER_TIMEOUT, 0.);
	p->last_activity = ev_now(EV_A);
	ev_timer_start(EV_A_ &p->stale_timer);

	list_add(&s->peers, &p->node);

	return p;
}

/* 'a' 'e' 'q' 'r' 't' 'y'
 * a: arguments for a query, dict.
 * e: list of a number and a string
 * q: string containing a queury name
 * 	"ping"
 * 	"find_node"
 * 	"get_peers"
 * 	"announce_peer"
 * r: the 'response' version of @a
 * t: a transaction ID, string of bytes.
 * y: "q", "r", or "e" to indicate the message type (query, response, or error)
 * 
 * = Non-standard =
 * v: 4 byte string where the first 2 bytes id the client and the second 2 the version.
 */
int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: %s <peer addr> <peer port>\n", argv[0]);
		return 1;
	}

	struct addrinfo *local = net_server_lookup("0", AF_INET, SOCK_DGRAM);
	if (!local)
		errx(1, "could not setup server addrinfo.");

	int sfd = socket(local->ai_family, local->ai_socktype,
			local->ai_protocol);
	if (sfd == -1)
		errx(1, "could not create socket.");

	struct sock s = {
		.buf = darray_new(),
		.peers = LIST_HEAD_INIT(s.peers),
	};
	load_random_id(&s.my_id);

	ev_io_init(&s.w, sock_cb, sfd, EV_READ);
	ev_io_start(EV_DEFAULT_ &s.w);

	struct peer *p = add_peer(EV_DEFAULT_ &s, argv[1], argv[2]);
	send_ping_to_peer(p);

	ev_run(EV_DEFAULT_ 0);

	return 0;
}
