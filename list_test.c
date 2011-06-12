#include "list.h"
#include <stdio.h>

struct foo {
	struct list_head list;
	int x;
};


int main(int argc, char **argv)
{

	LIST_HEAD(l);


	struct foo x, y, z;

	x.x = 1;
	y.x = 11;
	z.x = 145;

	list_add_tail(&x.list, &l);
	list_add_tail(&y.list, &l);
	list_add_tail(&z.list, &l);


	struct list_head *cur;
	list_for_each(&l, cur) {
		struct foo *c = list_entry(cur, struct foo, list);
		printf("%d\n", c->x);
	}

	return 0;
}
