/* Another kademlia variant */
#include <stdio.h>

#include <ccan/compiler/compiler.h>
#include <ccan/darray/darray.h>
#include <ccan/err/err.h>
#include <ccan/list/list.h>
#include <ccan/net/net.h>

#include <rbtree/rbtree.h>

#include <penny/print.h>
#include <penny/mem.h>

#include <ev.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "ben.h"
#include "benr.h"

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

enum krpc_error {
	KE_GENERIC = 201,
	KE_SERVER = 202,
	KE_PROTOCOL = 203,
	KE_UNK_METHOD = 204
};

enum krpc_query {
	KQ_NONE,
	KQ_PING,
	KQ_FIND_NODE,
	KQ_GET_PEERS,
	KQ_ANNOUNCE_PEER,
};

enum krpc_type {
	KT_NONE,
	KT_QUERY,
	KT_RESPONSE,
	KT_ERROR
};

struct krpc_msg_error {
	intmax_t code;
	const char *str;
	size_t len;
};

struct krpc_msg_query {
	enum krpc_query query;

	char *id;
	size_t id_len;

	union {
		/* q.find_node */
		struct {
			const char *target;
			size_t target_len;
		} q_find_node;
		/* r.find_node */
		struct {
			const char *nodes;
			size_t nodes_len;
		} r_find_node;
		/* q.get_peers */
		struct {
			/* always 20bytes */
			const char *info_hash;
		} q_get_peers;
		/* r.get_peers */
		struct {
			char *token;
			size_t token_len;
			/* and either 'values' or 'nodes' */
		} r_get_peers;

		/* q.announce_peer */
		struct {
			bool implied_port;
			/* always 20bytes */
			const char *info_hash;

			char *token;
			size_t token_len;

			uint_least16_t port;
		} q_announce_peer;
	} args;
};

struct krpc_msg {
	/* y */
	enum krpc_type  type;

	/* t */
	char *trans_id;
	size_t trans_id_len;

	union {
		/* e */
		struct krpc_msg_error error;
		/* q | r */
		struct krpc_msg_query query;
	};
};

/* 'a' 'e' 'q' 'r' 't' 'v' 'y'
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

static int krpc_msg_parse_query(struct krpc_msg *msg, struct benr *q)
{
	struct benr_string s;
	int r = benr_as_string(q, &s);
	if (r < 0) {
		printf("'q' is not a string\n");
		return -1;
	}

	if (memeqstr(s.start, s.len, "ping")) {
		msg->query.query = KQ_PING;
	} else if (memeqstr(s.start, s.len, "find_node")) {
		msg->query.query = KQ_FIND_NODE;
	} else if (memeqstr(s.start, s.len, "get_peers")) {
		msg->query.query = KQ_GET_PEERS;
	} else if (memeqstr(value, len, "announce_peer")) {
		msg->query.query = KQ_ANNOUNCE_PEER;
	} else {
		printf("'q' has unrecognized value: '%.*s'\n", (int)s.len, s.start);
		return -1;
	}

	return 0;
}

static enum krpc_type krpc_parse_y(struct benr *y)
{
	struct benr_string s;
	r = benr_as_string(&v_y, &s);
	if (r < 0) {
		printf("'y' is not a string\n");
		return KT_NONE;
	}

	if (s.len != 1) {
		printf("'y' does not have length 1: %zu\n", s.len);
		return KT_NONE;
	}

	switch (*s.start) {
	case 'e':
		return KT_ERROR;
	case 'q':
		return KT_QUERY;
	case 'r':
		return KT_RESPONSE;
	default:
		printf("'y' has an unrecognized value: '%c'\n", *s.start);
		return KT_NONE;
	}
}

static int krpc_msg_parse(struct krpc_msg *msg, const void *data, size_t data_len)
{
	struct benr top;
	benr_init(&top, data, data_len);

	struct benr_dict top_d;
	int r = benr_as_dict(&top, &top_d);
	if (r < 0) {
		printf("Top level was not a dict\n");
		return r;
	}

	/* scan through the top level dict, look for the keys we care about */
	struct benr_dict_iter top_di;
	benr_dict_iter(&top_d, &top_di);

	/* notable entries */
	struct benr v_a = {0}, v_e = {0}, v_q = {0}, v_r = {0}, v_t = {0}, v_y = {0}, v_v = {0};

	for (;;) {
		struct benr k, v;
		r = benr_dict_iter_next(&top_di, &k, &v);
		if (r < 0) {
			printf("Ran out of entries\n");
			break;
		}

		struct benr_string ks;
		r = benr_as_string(&k, &ks);
		if (r < 0) {
			printf("Non string dict key?\n");
			continue;
		}

		if (ks.len == 1) {
			switch (*ks.start) {
			case 'a':
				v_a = v;
				break;
			case 'e':
				v_e = v;
				break;
			case 'q':
				v_q = v;
				break;
			case 'r':
				v_r = v;
				break;
			case 't':
				v_t = v;
				break;
			case 'y':
				v_y = v;
				break;
			case 'v':
				v_v = v;
				break;
			default:
				printf("unknown top level key: '%.*s'\n", (int)ks.len, ks.start);
			}
		} else {
			printf("unknown top level key: '%.*s'\n", (int)ks.len, ks.start);
		}
	}

	/* we should now have all the entries we care about, start parsing them */
	msg->type = krpc_parse_y(&v_y);
	if (msg->type == KT_NONE)
		return -1;

	if (msg->type == KT_ERROR) {
		/* WHOO, an error. read it and print and we're done! */
	}

	if (msg->type == KT_QUERY || msg->type == KT_RESPONSE) {
		r = krpc_msg_parse_query(msg, &v_q);
		if (r < 0)
			return r;
	}

	/* TODO: use the type to look for further elements */

	return -1;
}

static void encode_id(darray_char *d, bt_node_id *id)
{
	ben_string(d, id->id, sizeof(id->id));
}

static void krpc_gen_ping(darray_char *d, bt_node_id *id, const void *msg_id,
		size_t msg_id_len)
{
	ben_dict_begin(d);

	/* a = { "id" : MY_NODE_ID } */
	ben_string_c(d, "a");
	ben_dict_begin(d);
	ben_string_c(d, "id");
	encode_id(d, id);
	ben_dict_end(d);

	/* q = "ping" */
	ben_string_c(d, "q");
	ben_string_c(d, "ping");

	/* t = SOME_ID */
	ben_string_c(d, "t");
	ben_string(d, msg_id, msg_id_len); /* some ID */

	/* y = "q" */
	ben_string_c(d, "y");
	ben_string_c(d, "q");

	ben_dict_end(d);
}

static void krpc_gen_ping_response(darray_char *d, bt_node_id *my_id,
		const void *msg_id, size_t msg_id_len)
{
	ben_dict_begin(d);

	/* r */
	ben_string_c(d, "r");
	ben_dict_begin(d);
	ben_string_c(d, "id");
	encode_id(d, my_id);
	ben_dict_end(d);

	/* t = SOME_ID */
	ben_string_c(d, "t");
	ben_string(d, msg_id, msg_id_len); /* some id */

	/* y */
	ben_string_c(d, "y");
	ben_string_c(d, "r");

	ben_dict_end(d);
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
	krpc_gen_ping(&s->buf, &s->my_id, "tt", 2);

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

/* 'a' 'e' 'q' 'r' 't' 'v' 'y'
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
