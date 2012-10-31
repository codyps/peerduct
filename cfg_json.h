#ifndef CFG_JSON_H_
#define CFG_JSON_H_

#include "kademlia.h"

#include "yajl/yajl_tree.h"

struct list_head {
	struct list_head *prev, *next;
};

struct config {
	yajl_val root;
	struct list_head peers;
};

int  config_load   (struct config *cfg, char *config_file);
void config_destroy(struct config *cfg);
int  config_get_id (struct config *cfg, struct kademlia_id *id);
#endif
