/*
 * Requires Sphinx
 *
 * http://arxiv.org/pdf/1507.05724v1.pdf
 *
 */

/*
 * each router has this
 */
struct hornet_router {
	/* local secret to encrypt the exported per-session state */
};

/* hornet forwarding segment 
 * 
 * source generates this
 */
struct hornet_fs {

};

/* hornet anonymous header, composed of multiple `fs`
 *
 * source generates this
 */
struct hornet_ahdr {

};


struct hornet {
	unsigned k; /* security parameter, paper uses 128 */


	/* G, a prime order cyclic group of order q (q ~ 2**(2k)), which satisfies the decisional dh assumption */
	/* G*, a set of non-identity elements in G */
	/* g, a generator of G */

	unsigned r; /* maximum length of a path */
};
