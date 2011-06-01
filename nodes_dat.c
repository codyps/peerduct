#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>

#include "nodes_dat.h"

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <endian.h> /* le32toh, le16toh */
#include <arpa/inet.h> /* htons */

int nd_parse_init(struct nd_parse_ctx *npc)
{
	npc->head_pos      = 0;
	npc->header_parsed = false;
	/* head ?? */
	npc->version       = 0;
	npc->peers         = NULL;
	npc->tail          = &(npc->peers);
	npc->nr_consumed   = 0;
	npc->nr_total      = 0;
	npc->error         = 0;
	/* cur_e ?? */
	npc->cur_e_pos     = 0;

	return 0;
}

#define le16ton(x) htons(le16toh(x))

static struct peer *mk_kad_peer(struct nd_entry *entry)
{
	struct kad_peer *kp = malloc(sizeof(*kp));
	if (!kp)
		return NULL;

	memcpy(kp->client_id, entry->client_id, sizeof(kp->client_id));
	memcpy(kp->kad_udp_key, entry->kad_udp_key, sizeof(kp->kad_udp_key));

	kp->version = entry->version;
	kp->verified = entry->verified;

	kp->udp.sin_family = AF_INET;
	kp->tcp.sin_family = AF_INET;

	kp->udp.sin_port = le16ton(entry->udp_port);
	kp->tcp.sin_port = le16ton(entry->tcp_port);

	kp->udp.sin_addr.s_addr = le16ton(entry->ip_addr);
	kp->tcp.sin_addr.s_addr = le16ton(entry->ip_addr);

	kp->peer.type = PT_KAD;
	kp->peer.next      = NULL;

	return &kp->peer;
}


int nd_parse_proc(struct nd_parse_ctx *npc, void *buf, size_t len)
{
	size_t consumed = 0; /* number of bytes in buf consumed by
				the current function call */
	
	if (npc->error < 0)
		return npc->error;

	if (len == 0)
		return consumed;

	if (!npc->header_parsed) {
		size_t cpy_sz = MIN(len, sizeof(npc->head) - npc->head_pos);
		memcpy( (char *)&(npc->head) + npc->head_pos, buf, cpy_sz);
		buf += cpy_sz;
		len -= cpy_sz;
		consumed += cpy_sz;
		npc->head_pos += cpy_sz;

		/* we may have either consumed all the data, or finished
		 * populating the header. */
		if (len == 0)
			return consumed;
		
		/* at this point, header fully is populated */
		if (npc->head.zero || le32toh(npc->head.version) != 2) {
			npc->error = -EINVAL;
			return npc->error;
		}

		npc->nr_total = le32toh(npc->head.nr);
		npc->header_parsed = true;
	}

	uint32_t nr_total = npc->nr_total;
	uint32_t nr;
	size_t   pos = npc->cur_e_pos;
	for (nr = npc->nr_consumed; nr < nr_total; nr++) {
		char *start_cpy = (char *)&(npc->cur_e) + pos;
		size_t rem = sizeof(npc->cur_e) - pos;

		size_t cpy_len = MIN(rem, len);

		memcpy(start_cpy, buf, cpy_len);

		pos += cpy_len;

		if (likely(pos == sizeof(npc->cur_e))) {
			/* we filled this entry, deserialize it
			 * and continue */
			struct peer *p = mk_kad_peer(&npc->cur_e);
			if (!p) {
				/* failed allocation? set npc->error and
				 * return the consumed bytes. Error will be
				 * returned on next call */
				npc->error = -errno;
				npc->nr_consumed = nr;
				npc->cur_e_pos = 0;
				return consumed;
			}

			pos = 0;
			*(npc->tail) = p;
			npc->tail = &(p->next);

		} else {
			/* entry not filled but data done.
			 * update things for next time */
			npc->cur_e_pos = pos;
			npc->nr_consumed = nr;
			return consumed;
		}
	}

	/* we've read all the nodes in the file */
	return consumed;
}


struct peer *nd_parse_get_peers(struct nd_parse_ctx *npc)
{
	struct peer *p = npc->peers;
	npc->peers = NULL;
	npc->tail  = &(npc->peers);
	return p;
}


void nd_parse_destroy(struct nd_parse_ctx *npc)
{
	free_kad_peers(npc->peers);
	npc->peers = NULL;
	npc->tail = &(npc->peers);
}


