#include "benp.h"

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define debug(...) fprintf(stderr, __VA_ARGS__)

static
bool benp_eq(const struct benp_ev *ev, size_t ev_ct, const void *bytes, size_t byte_ct)
{
	struct benp_ctx ctx;
	benp_init(&ctx, bytes, byte_ct);

	size_t i;
	for (i = 0; i < ev_ct; i++) {
		struct benp_ev e1 = ev[i];
		struct benp_ev e2 = benp_next(&ctx);

		if (e1.kind != e2.kind) {
			debug("%zu: kind mismatch: %d != %d\n", i, e1.kind, e2.kind);
			return false;
		}

		switch (e1.kind) {
		case BENP_EV_DONE:
			/* FIXME: pointer, hard to compare */
			break;
		case BENP_EV_INTEGER:
			if (e1.data.integer != e2.data.integer) {
				debug("%zu: integer mismatch: %jd != %jd\n", i, e1.data.integer, e2.data.integer);
				return false;
			}
			break;
		case BENP_EV_UINT:
			if (e1.data.uint != e2.data.uint)
				return false;
			break;
		case BENP_EV_STRING:
			if (e1.data.string.len != e2.data.string.len) {
				debug("%zu: string len mismatch: %zu != %zu\n", i, e1.data.string.len, e2.data.string.len);
				return false;
			}
			if (memcmp(e1.data.string.start, e2.data.string.start, e1.data.string.len)) {
				debug("%zu: string data mismatch: '%.*s' != '%.*s'\n", i,
						(int)e1.data.string.len, (const char *)e2.data.string.start,
						(int)e2.data.string.len, (const char *)e2.data.string.start);
				return false;
			}
			break;
		case BENP_EV_DICT_START:
		case BENP_EV_LIST_START:
		case BENP_EV_X_END:
			break;

		case BENP_EV_ERROR_UNEXPECTED_EOF:
			break;
		case BENP_EV_ERROR_UNEXPECTED_BYTE:
			break;
		case BENP_EV_ERROR_STRING_TOO_LONG:
			break;
		case BENP_EV_ERROR_UNEXPECTED_BYTE_IN_INT:
			break;
		}
	}

	return true;
}

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define A(x) x, ARRAY_SIZE(x)
#define S(x) x, (ARRAY_SIZE(x) - 1)

int main(void)
{

	struct benp_ev a[] = {
		{ .kind = BENP_EV_INTEGER, .data.integer = 1 },
		{ .kind = BENP_EV_DONE }
	};
	assert(benp_eq(A(a), S("i1e")));

	struct benp_ev b[] = {
		{ .kind = BENP_EV_DICT_START },
		{ .kind = BENP_EV_LIST_START },
		{ .kind = BENP_EV_X_END },
		{ .kind = BENP_EV_X_END }
	};
	assert(benp_eq(A(b), S("dlee")));

	struct benp_ev c[] = {
		{ .kind = BENP_EV_DICT_START },
		{ .kind = BENP_EV_STRING, .data.string = { .start = "hello", .len = 5 } },
		{ .kind = BENP_EV_INTEGER, .data.integer = -1 },
		{ .kind = BENP_EV_X_END }
	};
	assert(benp_eq(A(c), S("d5:helloi-1ee")));
	return 0;
}
