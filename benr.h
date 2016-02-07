#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Operate on bencoded data directly, without any decoding
 */

enum benr_kind {
	BENR_KIND_STRING,
	BENR_KIND_INT,
	BENR_KIND_DICT,
	BENR_KIND_LIST,
};

struct benr_string {
	unsigned char *start;
	size_t len;
};

struct benr_int {
	intmax_t i;
};

struct benr_dict {
	void *start;
	size_t max_bytes;
};

struct benr_list {
	void *start;
	size_t max_bytes;
};

struct benr {
	enum benr_kind kind;
	union {
		struct benr_string s;
		struct benr_int i;
		struct benr_dict d;
		struct benr_list l;
	} data;
};

void *benr_init(struct benr *b, void *data, size_t data_bytes);

bool benr_is_string(struct benr *b);
bool benr_is_int(struct benr *b);
bool benr_is_dict(struct benr *b);
bool benr_is_list(struct benr *b);

int benr_as_string(struct benr *b, struct benr_string *s);
int benr_as_int(struct benr *b, struct benr_string *s);
int benr_as_dict(struct benr *b, struct benr_string *s);
int benr_as_list(struct benr *b, struct benr_string *s);


struct benr_dict_iter {

};

struct benr_list_iter {

};

void benr_list_iter(struct benr_list *l, struct benr_list_iter *i);
void benr_dict_iter(struct benr_dict *l, struct benr_dict_iter *i);

bool benr_list_iter_has_next(struct benr_list_iter *l);
bool benr_dict_iter_has_next(struct benr_dict_iter *l);

int benr_list_iter_next(struct benr_list_iter *l, struct benr *b);
int benr_dict_iter_next(struct benr_dict_iter *l, struct benr_string *key, struct benr *val);
