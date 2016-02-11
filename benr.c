#include "benr.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

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

static void
benr_next(struct benr *b, struct benr_ctx *ctx)
{
	struct benr_ctx lctx = *ctx;
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
		*ctx = lctx;
		return;
	}
	case 'd':
		ctx_adv(&lctx);
		*b = (struct benr){
			.kind = BENR_DICT,
			.data = { .d = { .ctx = {
				.start = lctx.start,
				.len = lctx.len,
			}}}
		};
		*ctx = lctx;
		return;
	case 'l':
		ctx_adv(&lctx);
		*b = (struct benr){
			.kind = BENR_LIST,
			.data = { .l = { .ctx = {
				.start = lctx.start,
				.len = lctx.len,
			}}}
		};
		*ctx = lctx;
		return;
	case 'e':
		ctx_adv(&lctx);
		*b = (struct benr){
			.kind = BENR_X_END
		};
		*ctx = lctx;
		return;
	default:
		if (!isdigit(c)) {
			*b = (struct benr){
				.kind = BENR_ERR_UNEXPECTED_BYTE,
				.data = { .error_loc = lctx.start }
			};
			return;
		}

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
			};
			return;
		}

		struct benr_string s = {
			.start = lctx.start,
			.len = b_len.data.u,
		};
		ctx_adv_n(&lctx, s.len);
		*b = (struct benr) {
			.kind = BENR_STRING,
			.data = { .s = s }
		};
		*ctx = lctx;
		return;
	}
}

void benr_init(struct benr *b, const void *data, size_t data_bytes)
{
	struct benr_ctx lctx = { .start = data, .len = data_bytes };
	benr_next(b, &lctx);

	/*
	 * Because we share benr_next() with internal functions, it
	 * might emit a BENR_X_END, which is not a real type we should
	 * return, so replace it with an unexpected byte.
	 */
	if (b->kind == BENR_X_END) {
		*b = (struct benr){
			.kind = BENR_ERR_UNEXPECTED_BYTE,
			.data = { .error_loc = data }
		};
	}
}

void benr_dict_iter(const struct benr_dict *l, struct benr_dict_iter *i)
{
	*i = (struct benr_dict_iter){
		.ctx = l->ctx
	};
}

void benr_list_iter(const struct benr_list *l, struct benr_list_iter *i)
{
	*i = (struct benr_list_iter){
		.ctx = l->ctx
	};
}

/*
 * Anything that does not fully advance the context when parsed in
 * benr_next()
 */
static bool
benr_kind_is_container(enum benr_kind kind)
{
	return kind == BENR_DICT || kind == BENR_LIST;
}

static bool
benr_kind_is_error(enum benr_kind kind)
{
	return kind >= BENR_ERR_FIRST;
}

MUST_USE
static int
ctx_adv_container(struct benr_ctx *ctx)
{
	struct benr_ctx lctx = *ctx;
	uintmax_t depth = 1;
	/* scan forward, looking for container start & end markers until we have 1 extra end marker */
	struct benr b;
	while (depth) {
		benr_next(&b, &lctx);
		if (benr_kind_is_container(b.kind)) {
			depth ++;
		} else if (b.kind == BENR_X_END) {
			depth --;
		} else if (benr_kind_is_error(b.kind)) {
			/* FIXME: better error reporting */
			return -b.kind;
		} else if (b.kind == BENR_NONE) {
			/* FIXME: better error reporting: in this case, we ended too soon */
			return -BENR_ERR_UNEXPECTED_EOF;
		}
	}
	*ctx = lctx;
	/* FIXME: better error reporting */
	return 0;
}

/* FIXME: after returning -1 once, will not return -1 again (both benr's will
 * be BENR_NONE instead) */
int benr_list_iter_next(struct benr_list_iter *l, struct benr *b)
{
	struct benr x;
	benr_next(&x, &l->ctx);
	if (x.kind == BENR_X_END) {
		l->ctx.len = 0;
		return -1;
	}
	/* FIXME: see benr_dict_iter_next() fixme */
	if (benr_kind_is_container(x.kind)) {
		int r = ctx_adv_container(&l->ctx);
		if (r < 0)
			return r;
	}

	*b = x;
	return 0;
}

/* FIXME: does not check the key type */
/* FIXME: after returning -1 once, will not return -1 again (both benr's will
 * be BENR_NONE instead) */
int benr_dict_iter_next(struct benr_dict_iter *l, struct benr *key, struct benr *val)
{
	struct benr k, v;
	benr_next(&k, &l->ctx);
	if (k.kind == BENR_X_END) {
		l->ctx.len = 0;
		return -1;
	}
	benr_next(&v, &l->ctx);
	if (v.kind == BENR_X_END) {
		l->ctx.len = 0;
		return -1;
	}

	/*
	 * FIXME: scanning like this causes us to potentially examine the
	 * same data byte many times over (with deeply nested containers).
	 * Instead of imeidately advancing, we could defer this scan and use
	 * the parsing done on inner data to shrink the amount of double
	 * scan.
	 */
	if (benr_kind_is_container(v.kind)) {
		int r = ctx_adv_container(&l->ctx);
		if (r < 0)
			return r;
	}

	*key = k;
	*val = v;
	return 0;
}

int benr_as_string(const struct benr *b, struct benr_string *s)
{
	if (b->kind != BENR_STRING)
		return -1;

	*s = b->data.s;
	return 0;
}

int benr_as_int(const struct benr *b, intmax_t *s)
{
	if (b->kind != BENR_INT)
		return -1;
	*s = b->data.i;
	return 0;
}

int benr_as_dict(const struct benr *b, struct benr_dict *s)
{
	if (b->kind != BENR_DICT)
		return -1;
	*s = b->data.d;
	return 0;
}

int benr_as_list(const struct benr *b, struct benr_list *s)
{
	if (b->kind != BENR_LIST)
		return -1;
	*s = b->data.l;
	return 0;
}
