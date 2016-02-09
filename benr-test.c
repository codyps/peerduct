#include "benr.h"
#include <assert.h>
#include <string.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define S(x) x, (ARRAY_SIZE(x) - 1)

/*
 * this is formulated as a debugging aid for confirming the library works properly.
 * In real life, bencode equality can compare the encoded data directly for more efficiency
 */
static bool
benr_eq(struct benr *a, struct benr *b)
{
	if (a->kind != b->kind)
		return false;

	switch (a->kind) {
	case BENR_INT:
		return a->data.i == b->data.i;
	case BENR_STRING:
		if (a->data.s.len != b->data.s.len)
			return false;
		return !memcmp(a->data.s.start, b->data.s.start, a->data.s.len);
	default:
		/* XXX: eventually fill in the others */
		return false;
	}
}

int main(void)
{
	struct benr b;

	/* integer */
	benr_init(&b, S("i1e"));
	assert(b.kind == BENR_INT);
	assert(b.data.i == 1);

	/* integer, negative multi digit */
	benr_init(&b, S("i-42561e"));
	assert(b.kind == BENR_INT);
	assert(b.data.i == -42561);

	/* list */
	benr_init(&b, S("li3ei6ei-1ee"));
	assert(b.kind == BENR_LIST);

	struct benr_list l;
	int r = benr_as_list(&b, &l);
	assert(r >= 0);

	struct benr_list_iter li;
	benr_list_iter(&l, &li);

	struct benr bl;
	r = benr_list_iter_next(&li, &bl);
	assert(r >= 0);

	assert(bl.kind == BENR_INT);
	assert(bl.data.i == 3);

	r = benr_list_iter_next(&li, &bl);
	assert(r >= 0);

	assert(bl.kind == BENR_INT);
	assert(bl.data.i == 6);

	r = benr_list_iter_next(&li, &bl);
	assert(r >= 0);

	assert(bl.kind == BENR_INT);
	assert(bl.data.i == -1);

	r = benr_list_iter_next(&li, &bl);
	assert(r <  0);

	/* string */
	benr_init(&b, S("5:hello"));
	assert(b.kind == BENR_STRING);

	struct benr_string s;
	r = benr_as_string(&b, &s);
	assert(r >= 0);

	assert(s.len == 5);
	assert(!memcmp("hello", s.start, s.len));

	/* dict */
	benr_init(&b, S("d3:onei1e5:seveni7ee"));

	struct benr_dict d;
	r = benr_as_dict(&b, &d);
	assert(r >= 0);

	struct benr_dict_iter di;
	benr_dict_iter(&d, &di);

	struct benr k, v;
	r = benr_dict_iter_next(&di, &k, &v);
	assert(r >= 0);

	assert(benr_eq(&(struct benr){
		.kind = BENR_STRING,
		.data = { .s = {
			.start = (void *)"one",
			.len = 3,
		}}
	}, &k));

	assert(v.kind == BENR_INT);
	assert(v.data.i == 1);

	r = benr_dict_iter_next(&di, &k, &v);
	assert(r >= 0);

	assert(benr_eq(&(struct benr){
		.kind = BENR_STRING,
		.data = { .s = {
			.start = (void *)"seven",
			.len = 5,
		}}
	}, &k));

	assert(v.kind == BENR_INT);
	assert(v.data.i == 7);

	r = benr_dict_iter_next(&di, &k, &v);
	assert(r < 0);

	/* nesting */
	/*               [           ]
	 *                [][	    ]
	 *                   [ ][  ]
	 */
	benr_init(&b, S("lded1:ali3eee"));

	r = benr_as_list(&b, &l);
	assert(r >= 0);

	benr_list_iter(&l, &li);

	struct benr b1;
	r = benr_list_iter_next(&li, &b1);
	assert(r >= 0);

	r = benr_as_dict(&b1, &d);
	assert(r >= 0);

	benr_dict_iter(&d, &di);
	struct benr b2, v2;
	r = benr_dict_iter_next(&di, &b2, &v2);
	assert(r < 0);

	r = benr_list_iter_next(&li, &b1);
	assert(r >= 0);

	r = benr_as_dict(&b1, &d);
	assert(r >= 0);

	benr_dict_iter(&d, &di);
	r = benr_dict_iter_next(&di, &b2, &v2);
	assert(r >= 0);

	assert(benr_eq(&(struct benr){
		.kind = BENR_STRING,
		.data = { .s = {
			.start = (void*)"a",
			.len = 1,
		}}
	}, &b2));

	struct benr_list l2;
	r = benr_as_list(&v2, &l2);
	assert(r >= 0);

	struct benr_list_iter li2;
	benr_list_iter(&l2, &li2);

	struct benr b3;
	r = benr_list_iter_next(&li2, &b3);
	assert(r >= 0);

	assert(b3.kind == BENR_INT);
	assert(b3.data.i == 3);

	r = benr_list_iter_next(&li2, &b3);
	assert(r < 0);

	r = benr_dict_iter_next(&di, &b2, &v2);
	assert(r < 0);

	r = benr_list_iter_next(&li, &b1);
	assert(r < 0);

	return 0;
}
