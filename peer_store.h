#ifndef PEER_STORE_H_
#define PEER_STORE_H_

/* XXX: need a way to load and store peers.
 * Bencoded/JSON/ed2k .met/xml?
 * Maybe use sqlite or tokyocabinet? (do we need this? will data set be large
 * and lookup times an important item?)
 */

struct met_parse_ctx {
};

/**
 * met_parse_init - sets up the met_parse_ctx for processing.
 *
 * @mpc - an uninitialized met_parse_ctx.
 *
 * @return - 0 on success, -1 on failure.
 */
int met_parse_init(struct met_parse_ctx *mpc);

/**
 * met_parse_proc - processes the given buffer. Indicates completion of the
 *                  parse based on the amount it indicates it consumed.
 *
 * @mpc - an initialized met_parse_ctx.
 * @buf - a buffer containing met data (which may continue the met data
 *        passed in a previous call to met_parse_proc).
 * @len - the length of @buf.
 *
 * @return - <0 on error. Otherwise, the number of bytes of buf read.
 *           If the parsing is still in progress, this will be equal
 *           to @len. If the parsing is completed, it will be some value
 *           less than @len.
 */
int met_parse_proc(struct met_parse_ctx *mpc, char *buf, size_t len);

/**
 * met_parse_get_peers - returns the current peers asociated with the met_parse_ctx,
 *                       and removes them from the ctx such that calling
 *                       met_parse_get_peers a second time without further
 *                       parsing taking place will return 0 peers.
 *
 * @mpc - the context from which the peers are removed.
 *
 * @return - a heap allocated list of peers.
 */
struct peer *met_parse_get_peers(struct met_parse_ctx *mpc);

/**
 * met_parse_destroy - free any data allocated in init or the parsing process.
 *
 * @mpc - ctx to be destroyed.
 */
void met_parse_destroy(struct met_parse_ctx *mpc);



struct bencode_parse_state {
};

/**
 * load_peers_from_bencode - what torrent programs use bencode for
 *                           peer storage? Do they all have the same layout?
 * 
 * @start - first byte in the met file.
 * @end   - last byte in the met file.
 * @p     - points to a valid struct peer * to fill in.
 *
 * @return - 0 on success, -1 on failure.
 */
int load_peers_from_bencode(char *start, char *end, struct peer **p);






#endif
