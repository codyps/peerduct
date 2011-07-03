#ifndef CFG_JSON_H_
#define CFG_JSON_H_

#include "yajl/yajl_tree.h"

struct list_head {
	struct list_head *prev, *next;
};

struct j_config {
	yajl_val root;

	char *id;

	struct list_head peers;
};

int  j_config_load(struct j_config *cfg, char *config_file);
void j_config_destroy(struct j_config *cfg);
#endif
