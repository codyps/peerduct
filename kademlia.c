
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
	int              contact_max; /**< maximum number of contacts that should be in this bucket */
	int              contact_ct;  /**< the number of contacts tracked */
	struct list_head contacts;    /**< a list of the tracked contacts */
};

/**
 * kbucket_contact_insert - add a contact to a bucket
 * @bucket: the bucket to add a contact to
 * @contact: the contact to add. Data from this is copied, caller is
 *           responsible for the passed contacts memory managment.
 *
 * Returns 0 when the contact was inserted successfuly. -1 when a failure to
 * insert occoured. 1 when the contact already exsisted.
 */
int kbucket_contact_insert(struct kademlia_bucket *bucket,
		struct kademlia_contact const *contact)
{
	struct kademlia_contact *c = find_contact(bucket, contact);
	if (c) {
		list_del(c->list, bucket->contacts);
	} else {
		if (bucket->contact_ct >= bucket->contact_max)
			return -1;

		c = copy_contact(contact);
		if (!c)
			return -2;
		bucket->contact_ct ++;
	}

	list_add(c->list, bucket->contacts);
	return 0;
}

/**
 * struct kademlia_node - a unique member of the network, us.
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
