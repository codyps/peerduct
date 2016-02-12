#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MUST_USE __attribute__((warn_unused_result))

/*
 * benr - read bencoded data directly, without any intermediate data decoding
 *        or copying
 *
 * Note that iteration over items which are themselves containers will result
 * in the entire contents of the item being examined to locate the next item of
 * the iteration. This means that heavily nested structures will cause us to
 * read (and parse) the same bencoded data multiple times.
 *
 * In the worst case, assuming one iterates infinitely deep & the entire
 * bencoded message is composed of container entries and exits, given a message
 * of N bytes we will do:
 *
 * Total byte reads = R
 * R(N) = N + (N - 2) + (N - 2 - 2) + (N - 6) ...
 * R(N) = 2 + 4 + 6 + 8 ... + N
 *
 * r_n = r_(n
 *
 * R(N) = for (i = 1; i <= N/2; i++) 2*i
 * R(N) = ((N/2)**2 * ((N/2)**2 + 1))
 *
 * R(N) = O(N**2)
 *
 * So, kind of not so great.
 *
 * How could we avoid this?
 *
 * - Store information on lower iterations
 *	- requires dynamic allocation
 * - Modify the bencoded data to store parsing info
 *	- encoding for containers is probably too compact
 * - If users do depth-first decoding, we could have deeper decoders add
 *   information to upper decoders
 *	- Enlarges struct benr
 *	- Makes lifetimes _really_ complicated (need to keep intermediate benr structures around longer)
 *
 * We don't do any of the above to keep this library simple & to avoid the downsides noted
 *
 */

enum benr_kind {
	/* TODO: consider moving this to a return value? */
	BENR_NONE,

	/* real types */
	BENR_STRING,
	BENR_INT,
	BENR_DICT,
	BENR_LIST,

	/* internal */
	BENR_UINT,
	BENR_X_END,

	/* error-likes */

	/* FIXME: these should really be pushed into return values or
	 * some other error mechanism seperate from values. This mixture,
	 * while condensing the code, makes the API of
	 * benr_dict_iter_next() weird. */

	BENR_ERR_FIRST,
	/* all items after this *MUST* be errors */
	BENR_ERR_UNEXPECTED_BYTE = BENR_ERR_FIRST,
	BENR_ERR_UNEXPECTED_EOF,
	BENR_ERR_UNEXPECTED_BYTE_IN_INT,
	BENR_ERR_UNEXPECTED_EOF_IN_INT,
	BENR_ERR_STRING_TOO_LONG,
	BENR_ERR_KEY_IS_NOT_STRING,
};

struct benr_string {
	const unsigned char *start;
	size_t len;
};

struct benr_ctx {
	const void *start;
	size_t len;
};

struct benr_dict {
	/* note that ctx.len is max bytes, not strict bound */
	struct benr_ctx ctx;
};

struct benr_list {
	/* note that ctx.len is max bytes, not strict bound */
	struct benr_ctx ctx;
};

struct benr {
	enum benr_kind kind;
	union {
		struct benr_string s;
		intmax_t i;
		struct benr_dict d;
		struct benr_list l;

		/* error-likes */
		const void *error_loc;
		uintmax_t error_len;
		enum benr_kind error_kind;

		/* internal */
		uintmax_t u;
	} data;
};

void benr_init(struct benr *b, const void *data, size_t data_bytes);

MUST_USE
int benr_as_string(const struct benr *b, struct benr_string *s);

MUST_USE
int benr_as_int(const struct benr *b, intmax_t *s);

MUST_USE
int benr_as_dict(const struct benr *b, struct benr_dict *s);

MUST_USE
int benr_as_list(const struct benr *b, struct benr_list *s);


struct benr_dict_iter {
	struct benr_ctx ctx;
};

struct benr_list_iter {
	struct benr_ctx ctx;
};

void benr_list_iter(const struct benr_list *l, struct benr_list_iter *i);
void benr_dict_iter(const struct benr_dict *l, struct benr_dict_iter *i);

MUST_USE
int benr_list_iter_next(struct benr_list_iter *l, struct benr *b);

MUST_USE
int benr_dict_iter_next(struct benr_dict_iter *l, struct benr *key, struct benr *val);

#define BENR_INIT_NONE (struct benr){ \
	.kind = BENR_NONE, \
	.data.u = 0, \
}

MUST_USE
static inline
bool benr_is_error(struct benr *b)
{
	return b->kind >= BENR_ERR_FIRST;
}

