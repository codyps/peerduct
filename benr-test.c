#include "benr.h"
#include <assert.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define S(x) x, (ARRAY_SIZE(x) - 1)


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

	/* dict */

	return 0;
}
