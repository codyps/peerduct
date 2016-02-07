#include "benp.h"
#include <ctype.h>

static
char ctx_peek(struct benp_ctx *ctx)
{
	return *(char *)ctx->data.start;
}

static
void ctx_adv(struct benp_ctx *ctx)
{
	ctx->data.len --;
	ctx->data.start ++;
}

static
void ctx_adv_n(struct benp_ctx *ctx, size_t n)
{
	ctx->data.len -= n;
	ctx->data.start += n;
}

static
char ctx_next(struct benp_ctx *ctx)
{
	char b = ctx_peek(ctx);
	ctx_adv(ctx);
	return b;
}

static
struct benp_ev _benp_next_uint(struct benp_ctx *ctx, char end)
{
	uintmax_t v = 0;
	struct benp_ctx lctx = *ctx;
	while (lctx.data.len) {
		char it = ctx_next(&lctx);
		if (it == end) {
			*ctx = lctx;
			return (struct benp_ev){
				.kind = BENP_EV_UINT,
				.data = {
					.uint = v
				}
			};
		}
		if (!isdigit(it))
			return (struct benp_ev){
				.kind = BENP_EV_ERROR_UNEXPECTED_BYTE_IN_INT,
				.data = {
					.error_loc = lctx.data.start - 1,
				}
			};
		/* FIXME: detect overflow here */
		v *= 10;
		v += it - '0';
	}

	return (struct benp_ev){
		.kind = BENP_EV_ERROR_UNEXPECTED_EOF,
	};
}

struct benp_ev benp_next(struct benp_ctx *ctx)
{
	struct benp_ctx lctx = *ctx;
	if (lctx.data.len) {
		char fst = ctx_peek(&lctx);
		switch (fst) {
		case 'i': {
			ctx_adv(&lctx);
			if (!lctx.data.len)
				return (struct benp_ev){
					.kind = BENP_EV_ERROR_UNEXPECTED_EOF
				};
			char d = ctx_peek(&lctx);
			int sign = 1;
			if (d == '-') {
				sign = -1;
				ctx_adv(&lctx);
			}

			struct benp_ev e = _benp_next_uint(&lctx, 'e');
			if (e.kind != BENP_EV_UINT)
				return e;

			*ctx = lctx;
			e.data.integer = e.data.uint * sign;
			e.kind  = BENP_EV_INTEGER;
			return e;
		}
		case 'l':
			ctx_adv(ctx);
			return (struct benp_ev){
				.kind = BENP_EV_LIST_START,
			};
		case 'd':
			ctx_adv(ctx);
			return (struct benp_ev) {
				.kind = BENP_EV_DICT_START,
			};
		case 'e':
			ctx_adv(ctx);
			return (struct benp_ev){
				.kind = BENP_EV_X_END,
			};
		default:
			if (isdigit(fst)) {
				/* string! */
				struct benp_ev e = _benp_next_uint(&lctx, ':');
				if (e.kind != BENP_EV_UINT)
					return e;

				if ((e.data.uint > SIZE_MAX) || (e.data.uint > lctx.data.len))
					return (struct benp_ev){
						.kind = BENP_EV_ERROR_STRING_TOO_LONG,
						.data = {
							.error_len = e.data.uint,
						}
					};

				struct benp_bytes s = {
					.start = lctx.data.start,
					.len = e.data.uint,
				};

				ctx_adv_n(&lctx, s.len);
				*ctx = lctx;
				return (struct benp_ev){
					.kind = BENP_EV_STRING,
					.data = {
						.string = s
					}
				};
			} else {
				/* junk!, emit an error */
				return (struct benp_ev){
					.kind = BENP_EV_ERROR_UNEXPECTED_BYTE,
					.data = {
						.error_loc = lctx.data.start
					}
				};
			}
		}
	} else {
		return (struct benp_ev){
			.kind = BENP_EV_DONE,
		};
	}
}
