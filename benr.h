#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MUST_USE __attribute__((warn_unused_result))

/*
 * Operate on bencoded data directly, without any decoding
 */

enum benr_kind {
	/* TODO: consider moving these to return values? */
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
