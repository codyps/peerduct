#pragma once

/*
 * event-based parser choices:
 * 1. callback on event, different callback for each event
 * 2. return on event, return value contains a union of possible values
 *
 * #1 results in indirect calls, which are slow
 * #2 results in needing to use a tagged union, which C doesn't have direct support for (can it be faked?)
 * #2 requires we do an external switch in place of the indirect calls, which isn't ideal
 */


#include <stdlib.h>
#include <stdint.h>

struct benp_bytes {
	const void *start;
	size_t len;
};

struct benp_ctx {
	/*
	 * TODO: we don't track list_start/list_end and dict_start/dict_end to
	 * enforce well-formedness of the bencoded data. It seems like we could
	 * apply a wrapper on top of this to supply that verification.
	 */
	struct benp_bytes data;
};

enum benp_ev_e {
	/*
	 * data: none
	 */
	BENP_EV_DONE, /* 0 */

	/*
	 * data: @integer - a value
	 */
	BENP_EV_INTEGER, /* 1 */

	/*
	 * data: @string - a string
	 */
	BENP_EV_STRING, /* 2 */

	/*
	 * data: none
	 */
	BENP_EV_LIST_START, /* 3 */

	/*
	 * data: none
	 */
	BENP_EV_DICT_START, /* 4 */

	/*
	 * data: none
	 */
	BENP_EV_X_END, /* 5 */

	/*
	 * Not provided due to lack of tracking support:
	 * BENP_EV_DICT_KEY,
	 * BENP_EV_LIST_END,
	 * BENP_EV_DICT_END,
	 */

	/*
	 * error like events
	 */
	/*
	 * we reached the end of the data file in the middle of parsing some piece of data
	 * On return, ctx is unchanged.
	 *
	 * data: none
	 */
	BENP_EV_ERROR_UNEXPECTED_EOF, /* 6 */

	/*
	 * While looking for a type marker (for the next item), an unexpected byte was read.
	 * This can mean we're at the end of our input if their is non-bencoded
	 * data that follows it.
	 *
	 * On return, ctx is unchanged.
	 *
	 * data: @error_loc - location of the unexpected data
	 */
	BENP_EV_ERROR_UNEXPECTED_BYTE, /* 7 */

	/*
	 * while parsing a string, the length was too long
	 * On return, ctx is unchanged.
	 *
	 * data: @error_len - encoded length that was too long
	 */
	BENP_EV_ERROR_STRING_TOO_LONG, /* 8 */

	/*
	 * While parsing an integer, an unexpect (non-numeric) byte was encountered.
	 *
	 * data: @error_loc - location of the unexpected data
	 */
	BENP_EV_ERROR_UNEXPECTED_BYTE_IN_INT, /* 9 */

	/* internal */

	/*
	 * data: @uint - a value
	 */
	BENP_EV_UINT, /* 10 */
};

struct benp_ev {
	enum benp_ev_e kind;
	union {
		/* FIXME: no maximum is specified in the spec, we should
		 * endevour to support unbounded integers
		 */
		intmax_t integer;

		/*
		 * These strings/keys are owned by the context that returned
		 * them, and should not be freed or modified.
		 */
		struct benp_bytes string;
		struct benp_bytes key;

		/*
		 * no data for:
		 * - list_start
		 * - list_end
		 * - dict_start
		 * - dict_end
		 */

		/* errors */
		size_t error_len;
		const void *error_loc;

		/* internal */
		uintmax_t uint;
	} data;
};

/**
 * benp_init - initialize a benp context for parsing bencode contained in @data
 * ctx: the context to initialize
 * data: the location of bencode to parse
 * data_len: the length in bytes of @data
 *
 * This only borrows data into @ctx, it neither takes ownership nor copies the data.
 * Ensure you do not change or free or otherwise invalidate data while you are
 * utilizing @ctx or values returned from it.
 *
 * @data must be non-null
 * @ctx must be non-null
 *
 * Does not fail.
 */
static inline
void benp_init(struct benp_ctx *ctx, const void *data, size_t data_len)
{
	*ctx = (struct benp_ctx){
		.data = {
			.start = data,
			.len = data_len
		}
	};
}

/**
 * benp_next - get the next "event" or "token" from the context
 * ctx: the context to grap an event from
 *
 * Returns an event & advances the context
 */
struct benp_ev benp_next(struct benp_ctx *ctx);
