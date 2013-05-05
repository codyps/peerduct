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

struct krpc_msg {
	enum krpc_type  type;
	enum krpc_query query;

	long long error_code;
	char *error;
	size_t error_len;

	char *id;
	size_t id_len;

	char *trans_id; /* "t" */
	size_t trans_id_len;

	char *nodes;
	size_t nodes_len;

	char *target;
	size_t target_len;

	char *info_hash;
	size_t info_hash_len;

	char *token;
	size_t token_len;
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

enum ben_type {
	BT_NONE,
	BT_LIST,
	BT_DICT
};

struct krpc_msg_parse_level {
	enum ben_type type;
	unsigned idx;

	/* only used for dicts */
	char *key;
	size_t key_len;
};

struct krpc_msg_parse {
	struct krpc_msg *msg;

	struct krpc_msg_parse_level levels[32];
	int depth;
};

static bool kmp_depth_is_dict(struct krpc_msg_parse *kmp)
{
	return kmp->levels[kmp->depth] == BT_DICT;
}

static unsigned kmp_idx(struct krpc_msg_parse *kmp)
{
	return kmp->levels[kmp->depth].idx;
}

static int kmp_depth_inc(struct krpc_msg_parse *kmp, bool is_dict)
{
	kmp->depth++;
	if (kmp->depth >= ARRAY_SIZE(kmp->levels))
		return -1;
	if (is_dict)
		kmp->levels[kmp->depth]->type = BT_DICT;
	else
		kmp->levels[kmp->depth]->type = BT_LIST;
	kmp->levels[kmp->depth]->idx = 0;
	return 0;
}

static int kmp_depth_dec(struct krpc_msg_parse *kmp, bool is_dict)
{
	if (!kmp->depth)
		return -1;
	bool was_dict = kmp_depth_is_dict(kmp);
	if (was_dict != is_dict)
		return -1;

	kmp->depth--;
	return 0;
}

static void kmp_idx_inc(struct krpc_msg_parse *kmp)
{
	kmp->levels[kmp->depth]->idx ++;
}

static int krpc_msg_parse_type(struct krpc_msg_parse *kmp,
		char *value, size_t len)
{
	if (len != 1)
		return -1;
	if (kmp->msg->type != KT_NONE)
		return -1;

	int v = *value;
	switch (v) {
	case 'e':
		v = KT_ERROR;
		break;
	case 'q':
		v = KT_QUERY;
		break;
	case 'r':
		v = KT_RESPONSE;
		break;
	default:
		return -1;
	}

	kmp->msg->type = v;
	return 0;
}

static int krpc_msg_parse_query(struct krpc_msg_parse *kmp,
		char *value, size_t len)
{
	if (kmp->msg->query != KQ_NONE)
		return -1;

	if (memeqstr(value, len, "ping")) {
		kmp->msg->query = KQ_PING;
	} else if (memeqstr(value, len, "find_node")) {
		kmp->msg->query = KQ_FIND_NODE;
	} else if (memeqstr(value, len, "get_peers")) {
		kmp->msg->query = KQ_GET_PEERS;
	} else if (memeqstr(value, len, "announce_peer")) {
		kmp->msg->query = KQ_ANNOUNCE_PEER;
	} else
		return -1;
}

static int krpc_msg_parse_trans_id(struct krpc_msg_parse *kmp, char *value, size_t len)
{
	if (kmp->msg->trans_id)
		return -1;

	kmp->msg->trans_id = value;
	kmp->msg->trans_id_len = len;

	return 0;
}

static int krpc_msg_parse_string_arg(struct krpc_msg_parse *kmp, char *value, size_t len)
{
	if (memeqstr(value, len, "id")) {

	} else if (memeqstr(value, len, "info_hash")) {

	} else if (memeqstr(value, len, "target")) {

	} else if (memeqstr(value, len, "implied_port")) {

	} else if (memeqstr(value, len, "port")) {

	}
}

static int krpc_msg_parse_response(

static int krpc_msg_parse_string(void *ctx, char *value, size_t length)
{
	struct krpc_msg_parse *kmp = ctx;
	struct krpc_msg_parse_level *l = &kmp->levels[kmp->depth];
	int ret = 0;
	if (kmp->depth == 0)
		return -1;

	if (kmp->depth == 1 && kmp_depth_is_dict(kmp)) {
		if (l->key_len != 1)
			goto out;

		switch (*l->key) {
		case 'q':
			ret = krpc_msg_parse_query(kmp, value, length);
			break;
		case 't':
			ret = krpc_msg_parse_trans_id(kmp, value, length);
			break;
		case 'y':
			ret =  krpc_msg_parse_type(kmp, value, length);
			break;
		case 'a':
		case 'e':
		case 'r':
			/* these are not supposed to be strings */
			return -1;
		/* case 'v': */
		/* ignore */
		}
	} else if (kmp->depth == 2) {
		if (!kmp_depth_is_dict(kmp)) {
			/* error? */
			if (l->key_len != 1 && *l->key != 'e')
				goto out;

			/* nothing but the 2nd elem can be a string */
			if (kmp_idx(kmp) != 1)
				return -1;

			ret = krpc_msg_parse_error_str(kmp, value, length);
		} else {
			struct krpc_msg_parse_level *pl =
				&kmp->levels[kmp->depth];
			if (pl->key_len != 1)
				goto out;
			switch (*pl->key) {
			case 'a':
				/* args */
				ret = krpc_msg_parse_args(kmp, value, length);
				break;
			case 'r':
				/* response */
				ret = krpc_msg_parse_response(kmp, value, length);
			}
		}
	} else if (kmp->depth == 3) {
		/* some types of args do this */
	}

out:
	kmp_idx_inc(kmp);
	return ret;
}

static int krpc_msg_parse_integer(void *ctx, long long value)
{
	struct krpc_msg_parse *kmp = ctx;
	if (kmp->depth == 0)
		return -1;

	/* XXX: unlike parse_string(), I don't bother erroring when
	 * and integer is used in an invalid field */

	/* the only place I care about an integer is in the "error" context */
	if (kmp->depth != 2
			|| kmp_depth_is_dict(kmp)
			|| kmp->levels[kmp->depth].idx != 0
			|| kmp->levels[kmp->depth - 1].key_len != 1
			|| kmp->levels[kmp->depth - 1].key != 'e')
		goto out;

	kmp->msg->error_code = value;

out:
	kmp_idx_inc(kmp);
	return 0;
}

static int krpc_msg_parse_list_start(void *ctx, char *value, size_t length)
{
	struct krpc_msg_parse *kmp = ctx;
	if (kmp->depth == 0)
		return -1;
	if (kmp_depth_inc(kmp, false))
		return -1;
	return 0;
}

static int krpc_msg_parse_list_end(void *ctx)
{
	struct krpc_msg_parse *kmp = ctx;
	if (kmp->depth == 0)
		return -1;
	if (kmp_depth_dec(kmp, false))
		return -1;
	kmp_idx_inc(kmp);
	return 0;
}

static int krpc_msg_parse_dict_start(void *ctx)
{
	struct krpc_msg_parse *kmp = ctx;
	if (kmp_depth_inc(kmp, true))
		return -1;
	return 0;
}

static int krpc_msg_parse_dict_key(void *ctx, char *key, size_t length)
{
	struct krpc_msg_parse *kmp = ctx;

	kmp->levels[kmp->depth].key = key;
	kmp->levels[kmp->depth].key_len = length;

	return 0;
}

static int krpc_msg_parse_dict_end(void *ctx)
{
	struct krpc_msg_parse *kmp = ctx;
	if (kmp_depth_dec(kmp, true))
		return -1;
	kmp_idx_inc(kmp);
	return 0;
}

struct tbl_callbacks krpc_msg_parse_callbacks {
	.integer = krpc_msg_parse_integer,
	.string  = krpc_msg_parse_string,
	.list_start = krpc_msg_parse_list_start,
	.list_end = krpc_msg_parse_list_end,
	.dict_start = krpc_msg_parse_dict_start,
	.dict_key = krpc_msg_parse_dict_key,
	.dict_end = krpc_msg_parse_dict_end,
};

static int krpc_msg_parse(struct krpc_msg *msg, const void *data, size_t data_len)
{
	struct krpc_msg_parse kmp = {
		.msg = msg,
	};

	return tbl_parse(data, data_len, &krpc_msg_parse_callbacks, &kmp);
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
