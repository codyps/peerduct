/* Another kademlia variant */

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

int main(int argc, char **argv)
{

}
