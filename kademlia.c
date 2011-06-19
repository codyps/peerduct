
/**
 * struct kademlia_contact - data to directly reach a node
 */
struct kademlia_contact {
	addr;
	port;
	id;
};

/**
 * struct kademlia_bucket - collection of contacts
 */
struct kademlia_bucket {

};

/**
 * struct kademlia_node - a unique member of the network
 */
struct kademlia_node {

};

/**
 * struct kademlia_ops - operations on a kademlia network
 */
struct kademlia_ops {
	ping;
	store;
	find_node;
	find_value;

	/* Extensions */
	delete; /**< Entangled */
};

/**
 * struct kademlia - a kademlia network
 *
 * Contents based on:
 * http://xlattice.sourceforge.net/components/protocol/kademlia/specs.html
 */
struct kademlia {
	int alpha;    /**< parralelism in network calls (?) */
	int k;        /**< number of contacts stored in a bucket. Should be even (?) */
	int key_size; /**< B */

	tExpire;      /**< the time after which a key/value pair expires; this
			is a time-to-live (TTL) from the original publication
			date */
	tRefresh;     /**< after which an otherwise unaccessed bucket must be
			refreshed */
	tReplicate;   /**< the interval between Kademlia replication events,
			when a node is required to publish its entire database
			*/
	tRepublish;   /**< the time after which the original publisher must
			republish a key/value pair */
};
