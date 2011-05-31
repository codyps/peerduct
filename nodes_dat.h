#ifndef NODES_DAT_H_
#define NODES_DAT_H_

#include <netinet/in.h> /* sockaddr_in */

#include "useful.h" /* __packed */
#include "peer.h" /* kad_peer */

/*
 * nodes.dat (v2) parsing.
 * 	used in amule >= 2.2.0 for KAD contacts.
 * 	used in emule (version?) for KAD contacts.
 *
 * doc - http://wiki.amule.org/index.php/Nodes.dat_file
 *
 * all fields are LE.
 */
struct nd_header {
	uint32_t zero;    /* always 0, used to differentiate from v1, which has
			     nr here. */
	uint32_t version; /* of the fileformat, always 2 */
	uint32_t nr;      /* number of nodes in file */
} __packed;

struct nd_entry {
	uint8_t client_id[16];
	uint32_t ip_addr;
	uint16_t tcp_port;
	uint16_t udp_port;
	uint8_t  version;  /* 0 = kad v1, non-zero indicates v2 and is a
			      featured bitmask */
	uint8_t  kad_udp_key[8]; 
	uint8_t  verified; /* non-zero value indicates verified. */
} __packed;

struct nd_parse_ctx {
	size_t len; /* bytes read so far */

	struct nd_header head;

	struct peer *peers;
	struct peer **tail;

	size_t nr; /* entrys read so far */
	int error; /* if less than zero, return as error and parse no more. */

	struct nd_entry cur_e;
	size_t cur_e_pos; /* byte offset within cur_e which is valid (partially
			     populated cur_e) */


};

/**
 * nd_parse_init - sets up the nd_parse_ctx for processing.
 *
 * @npc - an uninitialized nd_parse_ctx.
 *
 * @return - 0 on success, -1 on failure.
 */
int nd_parse_init(struct nd_parse_ctx *npc);

/**
 * nd_parse_proc - processes the given buffer. Indicates completion of the
 *                  parse based on the amount it indicates it consumed.
 *
 * @npc - an initialized nd_parse_ctx.
 * @buf - a buffer containing nd data (which may continue the nd data
 *        passed in a previous call to nd_parse_proc).
 * @len - the length of @buf.
 *
 * @return - <0 on error. Otherwise, the number of bytes of buf read.
 *           If the parsing is still in progress, this will be equal
 *           to @len. If the parsing is completed, it will be some value
 *           less than @len.
 */
int nd_parse_proc(struct nd_parse_ctx *npc, void *buf, size_t len);

/**
 * nd_parse_get_peers - returns the current peers asociated with the
 *                      nd_parse_ctx, and removes them from the ctx such that
 *                      calling nd_parse_get_peers a second time without
 *                      further parsing taking place will return 0 peers.
 *
 * @npc - the context from which the peers are removed.
 *
 * @return - a heap allocated list of peers.
 */
struct peer *nd_parse_get_peers(struct nd_parse_ctx *npc);

/**
 * nd_parse_destroy - free any data allocated in init or the parsing process.
 *                    This excludes any peers removed from the parse context
 *                    by a call to nd_parse_get_peers.
 *
 * @npc - ctx to be destroyed.
 */
void nd_parse_destroy(struct nd_parse_ctx *npc);

#endif
