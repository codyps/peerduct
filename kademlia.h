#ifndef KADEMLIA_H_
#define KADEMLIA_H_

#include <stdint.h>

struct kademlia_id {
	union {
		uint8_t  bytes[20];
		uint32_t dwords[5];
	};
};

#endif
