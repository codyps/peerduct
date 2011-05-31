#ifndef USEFUL_H_
#define USEFUL_H_

#include <stddef.h>

#define __packed __attribute__((packed))
#define MIN(x,y) ((x)>(y)?(y):(x))
#define likely(x) __builtin_expect(x,1)

#define container_of(ptr, type, member) ({ \
		const typeof( ((type *)0)->member ) *__mptr = (ptr); \
		(type *)( (char *)__mptr - offsetof(type,member) );})


#endif
