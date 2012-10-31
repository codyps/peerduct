
#include "cfg_json.h"
#include "yajl/yajl_tree.h"

#include <stdio.h>

#include <string.h> /* strlen */

static int base64_decode(void *dst, size_t dst_sz, const char *src, size_t src_sz)
{
	/* TODO: is there some good code for this? some standard lib? (openssl
	 * has it, though it seems a silly reason to pull in all of openssl) */

	return -1;
}

int config_get_id(struct config *cfg, struct kademlia_id *id)
{
	const char * path[] = { "id", NULL };
	yajl_val v = yajl_tree_get(cfg->root, path, yajl_t_string);
	if (v) {
		char *id_str = YAJL_GET_STRING(v);

		return base64_decode(id->bytes, sizeof(id->bytes), id_str, strlen(id_str));
	} else {
		return -1;
	}
}

static void get_peers(struct config *cfg)
{
	/* belh */
}

void config_destroy(struct config *cfg)
{
	yajl_tree_free(cfg->root);
}

int config_load(struct config *cfg, char *config_file)
{
	char file_data[65536];
	char errbuf[1024];

	file_data[0] = errbuf[0] = 0;

	FILE *fd = fopen(config_file, "rb");
	if (!fd) {
		fprintf(stderr, "failed to open file \"%s\"\n", config_file);
		return -1;
	}

	size_t rd = fread(file_data, 1, sizeof(file_data), fd);
	if ((rd == 0 && !feof(fd)) || ferror(fd)) {
		fprintf(stderr, "error encountered on file read\n");
		return -2;
	}
	
	cfg->root = yajl_tree_parse(file_data, errbuf, sizeof(errbuf));

	if (cfg->root == NULL) {
		fprintf(stderr, "parse error: ");
		if (strlen(errbuf))
			fprintf(stderr, " %s", errbuf);
		else
			fprintf(stderr, "unknown error");
		fprintf(stderr, "\n");
		return -3;
	}
	

	get_peers(cfg);

	return 0;
}
