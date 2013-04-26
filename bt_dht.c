/* Another kademlia variant */
#include <ccan/darray/darray.h>
#include <ccan/rbtree/rbtree.h>

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
	struct rb_tree buckets;
};

enum KRPC_ERROR {
	KE_GENERIC = 201,
	KE_SERVER = 202,
	KE_PROTOCOL = 203,
	KE_UNK_METHOD = 204
};

void dict_begin(darray_char *d)
{
	darray_append(buf, 'd');
}

void dict_end(darray_char *d)
{
	darray_append(buf, 'e');
}

/* 'y', 'q', 'r', 'e', 'a'
 * a: arguments for a query, dict.
 * e: list of a number and a string
 * q: string containing a queury name
 * 	"ping"
 * 	"find_node"
 * 	"get_peers"
 * 	"announce_peer"
 * r: the 'response' version of @a
 */
int main(int argc, char **argv)
{
	darray_char buf = darray_new();
	darray_



	return 0;
}
