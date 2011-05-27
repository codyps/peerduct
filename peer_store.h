#ifndef PEER_STORE_H_
#define PEER_STORE_H_

/* XXX: need a way to load and store peers.
 * Bencoded/JSON/ed2k .met/xml?
 * Maybe use sqlite or tokyocabinet? (do we need this? will data set be large
 * and lookup times an important item?)
 */

/** 
 * load_peers_from_met - given a buffer containing a met file (indicated by
 *                       @start and @end), populates @*p. MET is the file
 *                       format used by emule/ed2k.
 *
 * @start - first byte in the met file.
 * @end   - last byte in the met file.
 * @p     - points to a valid struct peer * to fill in.
 *
 * @return - 0 on success, -1 on failure.
 *
 */
int load_peers_from_met(char *start, char *end, struct peer **p);


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
