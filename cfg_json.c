
#include "cfg_json.h"
#include "yajl/yajl_tree.h"

#include <stdio.h>

#include <string.h> /* strlen */

static void get_id(struct j_config *cfg)
{
	const char * path[] = { "self", "id", NULL };
	yajl_val v = yajl_tree_get(cfg->root, path, yajl_t_string);
	if (v) {
		cfg->id = YAJL_GET_STRING(v);
		fprintf(stderr, "got id %s\n", cfg->id);
	} else {
		fprintf(stderr, "need to generate id");
	}
}

static void get_peers(struct j_config *cfg)
{
	/* belh */
}

void j_config_destroy(struct j_config *cfg)
{
	yajl_tree_free(cfg->root);
}

int j_config_load(struct j_config *cfg, char *config_file)
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
	

	get_id(cfg);
	get_peers(cfg);

	return 0;
}
