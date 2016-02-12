#ifndef BEN_H_
#define BEN_H_

#include <ccan/darray/darray.h>
#include <assert.h>
#include <limits.h>

/* TODO: provide checking for
 *  - improper dict ordering
 *  - not specing a string as the dict key
 * TODO: support output into a non-darray
 */

static inline void ben_dict_begin(darray_char *d)
{
	darray_append(*d, 'd');
}

static inline void ben_dict_end(darray_char *d)
{
	darray_append(*d, 'e');
}

static inline void ben_list_begin(darray_char *d)
{
	darray_append(*d, 'l');
}

static inline void ben_list_end(darray_char *d)
{
	darray_append(*d, 'e');
}

static inline void ben_string(darray_char *d, const char *bytes, size_t len)
{
	assert(len <= INT_MAX);
	darray_printf(*d, "%zu:%.*s", len, (int)len, bytes);
}

static inline void ben_integer(darray_char *d, long long value)
{
	darray_printf(*d, "i%llde", value);
}

static inline void ben_string_c(darray_char *d, const char *str)
{
	ben_string(d, str, strlen(str));
}

#endif
