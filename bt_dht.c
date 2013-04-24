/* Another kademlia variant */

struct bucket {
	struct rb_node node;
	unsigned min;
	unsigned max;
};

struct routing_table {
	size_t bucket_ct;
	struct rb_tree buckets;
};

int main(int argc, char **argv)
{

}
