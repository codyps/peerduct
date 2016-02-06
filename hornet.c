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
