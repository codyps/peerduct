#include "benr.h"

#include <ctype.h>

static
char ctx_peek(struct benr_ctx *ctx)
{
	return *(char *)ctx->start;
}

static
void ctx_adv(struct benr_ctx *ctx)
{
	ctx->len --;
	ctx->start ++;
}

static
void ctx_adv_n(struct benr_ctx *ctx, size_t n)
{
	ctx->len -= n;
	ctx->start += n;
}

static
char ctx_next(struct benr_ctx *ctx)
{
	char b = ctx_peek(ctx);
	ctx_adv(ctx);
	return b;
}

static void
benr_init_uint(struct benr *b, struct benr_ctx *ctx, char end)
{
	uintmax_t v = 0;
	struct benr_ctx lctx = *ctx;
	while (lctx.len) {
		char it = ctx_next(&lctx);
		if (it == end) {
			*ctx = lctx;
			*b =  (struct benr){
				.kind = BENR_UINT,
				.data = {
					.u = v
				}
			};
			return;
		}
		if (!isdigit(it)) {
			*b = (struct benr){
				.kind = BENR_ERR_UNEXPECTED_BYTE_IN_INT,
				.data = {
					.error_loc = lctx.start - 1,
				}
			};
			return;
		}
		/* FIXME: detect overflow here */
		v *= 10;
		v += it - '0';
	}

	*b =  (struct benr){
		.kind = BENR_ERR_UNEXPECTED_EOF_IN_INT,
	};
}

void benr_init(struct benr *b, void *data, size_t data_bytes)
{
	struct benr_ctx lctx = { .start = data, .len = data_bytes };
	if (!lctx.len) {
		*b = (struct benr){.kind = BENR_NONE};
		return;
	}

	char c = ctx_peek(&lctx);
	switch (c) {
	case 'i': {
		ctx_adv(&lctx);
		if (!lctx.len) {
			*b = (struct benr){
				.kind = BENR_ERR_UNEXPECTED_EOF
			};
			return;
		}
		char d = ctx_peek(&lctx);
		int sign = 1;
		if (d == '-') {
			sign = -1;
			ctx_adv(&lctx);
		}

		benr_init_uint(b, &lctx, 'e');
		if (b->kind != BENR_UINT)
			return;

		b->data.i = b->data.u * sign;
		b->kind = BENR_INT;
		return;
	}
	case 'd':
		*b = (struct benr){
			.kind = BENR_DICT,
			.data = { .d = { .ctx = {
				.start = data + 1,
				.len = data_bytes - 1,
			}}}
		};
		return;
	case 'l':
		*b = (struct benr){
			.kind = BENR_LIST,
			.data = { .l = { .ctx = {
				.start = data + 1,
				.len = data_bytes - 1,
			}}}
		};
		return;
	default:
		if (!isdigit(c)) {
			*b = (struct benr){
				.kind = BENR_ERR_UNEXPECTED_BYTE,
				.data = { .error_loc = data }
			};
			return;
		}

		/* TODO: parse string */
		struct benr b_len;
		benr_init_uint(&b_len, &lctx, ':');

		if (b_len.kind != BENR_UINT) {
			*b = b_len;
			return;
		}

		if ((b_len.data.u > SIZE_MAX) || (b_len.data.u > lctx.len)) {
			*b = (struct benr){
				.kind = BENR_ERR_STRING_TOO_LONG,
				.data = { .error_len = b_len.data.u }
			}
			return;
		}

		struct benr_string s = {
			.start = lctx.start,
			.len = b_len.data.u,
		};
		ctx_adv_n(&lctx, s.len);
		/* *ctx = lctx; */
		*b = (struct benr) {
			.kind = BENR_STRING,
			.data = { .s = s }
		};
		return;
	}
}
