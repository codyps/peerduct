#include "benr.h"
#include "benr.c"

#include <stdio.h>
#define debug(...) fprintf(stderr, __VA_ARGS__)

static
bool benr_eq(const struct benr *ev, size_t ev_ct, const void *bytes, size_t byte_ct)
{
	struct benr_ctx ctx = {
		.start = bytes,
		.len = byte_ct,
	};

	size_t i;
	for (i = 0; i < ev_ct; i++) {
		struct benr e1 = ev[i];
		struct benr e2;
		benr_next(&e2, &ctx);

		if (e1.kind != e2.kind) {
			debug("%zu: kind mismatch: %d != %d\n", i, e1.kind, e2.kind);
			return false;
		}

		switch (e1.kind) {
		case BENR_NONE:
			/* FIXME: pointer, hard to compare */
			break;
		case BENR_INT:
			if (e1.data.i != e2.data.i) {
				debug("%zu: integer mismatch: %jd != %jd\n", i, e1.data.i, e2.data.i);
				return false;
			}
			break;
		case BENR_UINT:
			if (e1.data.u != e2.data.u)
				return false;
			break;
		case BENR_STRING:
			if (e1.data.s.len != e2.data.s.len) {
				debug("%zu: string len mismatch: %zu != %zu\n", i, e1.data.s.len, e2.data.s.len);
				return false;
			}
			if (memcmp(e1.data.s.start, e2.data.s.start, e1.data.s.len)) {
				debug("%zu: string data mismatch: '%.*s' != '%.*s'\n", i,
						(int)e1.data.s.len, (const char *)e2.data.s.start,
						(int)e2.data.s.len, (const char *)e2.data.s.start);
				return false;
			}
			break;
		case BENR_DICT:
		case BENR_LIST:
		case BENR_X_END:
			break;

		default:
			debug("%zu: unhandled kind: %d\n", i, e1.kind);
			return false;
		}
	}

	return true;
}

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define A(x) x, ARRAY_SIZE(x)
#define S(x) x, (ARRAY_SIZE(x) - 1)

int main(void)
{

	struct benr a[] = {
		{ .kind = BENR_INT, .data.i= 1 },
		{ .kind = BENR_NONE, }
	};
	assert(benr_eq(A(a), S("i1e")));

	struct benr b[] = {
		{ .kind = BENR_DICT },
		{ .kind = BENR_LIST },
		{ .kind = BENR_X_END },
		{ .kind = BENR_X_END }
	};
	assert(benr_eq(A(b), S("dlee")));

	struct benr c[] = {
		{ .kind = BENR_DICT },
		{ .kind = BENR_STRING, .data.s = { .start = (const unsigned char*)"hello", .len = 5 } },
		{ .kind = BENR_INT, .data.i = -1 },
		{ .kind = BENR_X_END }
	};
	assert(benr_eq(A(c), S("d5:helloi-1ee")));

	struct benr d[] = {
		{ .kind = BENR_LIST },
		{ .kind = BENR_STRING, .data.s = { .start = (const unsigned char*)"e", .len = 1 } },
		{ .kind = BENR_X_END },
		{ .kind = BENR_NONE }
	};

	assert(benr_eq(A(d), S("l1:ee")));

	struct benr e[] = {
	/* 0 */	{ .kind = BENR_LIST }, /* #1 { */
	/* 1 */	{ .kind = BENR_DICT }, /* #2 { */
	/* 2 */	{ .kind = BENR_X_END },/* } dict, #2*/
	/* 3 */	{ .kind = BENR_DICT }, /* #3 { */
	/* 4 */	{ .kind = BENR_STRING, .data.s = {.start=(const unsigned char*)"e", .len=1}},
	/* 5 */	{ .kind = BENR_LIST }, /* #4 { */
	/* 6 */	{ .kind = BENR_INT, .data.i = 3 },
	/* 7 */	{ .kind = BENR_X_END },/* } list, #4 */
	/* 8 */	{ .kind = BENR_X_END },/* } dict, #3 */
	/* 9 */	{ .kind = BENR_X_END },/* } list, #1 */
	/* 10 */{ .kind = BENR_NONE },
	};

	assert(benr_eq(A(e), S("lded1:eli3eeee")));
	return 0;
}
