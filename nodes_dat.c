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
#define le32ton(x) htonl(le32toh(x))

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

static struct peer *mk_kad_peer_v0(struct nd_entry_v0 *entry)
{
	struct kad_peer *kp = malloc(sizeof(*kp));
	if (!kp)
		return NULL;

	memcpy(kp->client_id, entry->client_id, sizeof(kp->client_id));
	memset(kp->kad_udp_key, 0, sizeof(kp->kad_udp_key));

	kp->version = 0;
	kp->verified = 0;

	kp->udp.sin_family = AF_INET;
	kp->tcp.sin_family = AF_INET;

	kp->udp.sin_port = le16ton(entry->udp_port);
	kp->tcp.sin_port = le16ton(entry->tcp_port);

	/* stored as BE aka Network byte order.
	 * sockaddr_in expects network byte order.
	 * all is well. */
	kp->udp.sin_addr.s_addr = entry->ip_addr;
	kp->tcp.sin_addr.s_addr = entry->ip_addr;

	kp->peer.type = PT_KAD;
	kp->peer.next = NULL;

	return &kp->peer;
}


/* do not call if npc->header_parsed is true */
static int header_parser(struct nd_parse_ctx *npc, void *buf, size_t len)
{
	size_t consumed = 0;

	if (npc->head_pos < sizeof(uint32_t)) {
		size_t cpy_sz = MIN(len, sizeof(uint32_t) - npc->head_pos);
		memcpy( (char *)&(npc->head) + npc->head_pos, buf, cpy_sz);
		npc->head_pos += cpy_sz;
		consumed += cpy_sz;

		if (npc->head_pos >= sizeof(uint32_t)) {
			if (npc->head.zero) {
				npc->version = 0;
				npc->nr_total = le32toh(npc->head.zero);
				npc->header_parsed = true;
				return consumed;
			} else {
				buf += cpy_sz;
				len -= cpy_sz;
			}
		}

	}
	
	/* this is not an else case of the first if as the value being compared
	 * against could be changed within the first if. */
	if (npc->head_pos >= sizeof(uint32_t)) {
		size_t cpy_sz = MIN(len, sizeof(npc->head) - npc->head_pos);
		memcpy( (char *)&(npc->head) + npc->head_pos, buf, cpy_sz);
		npc->head_pos += cpy_sz;
		consumed += cpy_sz;

		if (npc->head_pos >= sizeof(npc->head)) {
			npc->version = le32toh(npc->head.version);
			npc->nr_total = le32toh(npc->head.nr);
			npc->header_parsed = true;
			return consumed;
		}

	}

	return consumed;
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
		int r = header_parser(npc, buf, len);

		if (!npc->header_parsed)
			return r;

		buf += r;
		len -= r;
		consumed += r;
	}

	/* if we pass the above if, the header must be parsed.
	 * Thus, version and  nr_total will be populated. */

	size_t goal_len;
	if (npc->version == 0)
		goal_len = sizeof(npc->cur_e_v0);
	else if (npc->version == 2)
		goal_len = sizeof(npc->cur_e);
	else {
		npc->error = -EINVAL;
		return npc->error;
	}

	uint32_t nr_total = npc->nr_total;
	uint32_t nr;
	size_t   pos = npc->cur_e_pos;
	for (nr = npc->nr_consumed; nr < nr_total; nr++) {
		char *start_cpy = (char *)&(npc->cur_e) + pos;
		size_t rem = goal_len - pos;

		size_t cpy_len = MIN(rem, len);

		memcpy(start_cpy, buf, cpy_len);

		pos += cpy_len;
		len -= cpy_len;
		buf += cpy_len;

		if (likely(pos == goal_len)) {
			/* we filled this entry, deserialize it
			 * and continue */
			struct peer *p = npc->version ?
				mk_kad_peer(&npc->cur_e) :
				mk_kad_peer_v0(&npc->cur_e_v0);
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
			consumed += cpy_len;

		} else {
			/* entry not filled but data done.
			 * update things for next time */
			npc->cur_e_pos = pos;
			npc->nr_consumed = nr;
			return consumed + cpy_len;
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


