#ifndef __BALANCER__
#define __BALANCER__
#include <inttypes.h>
#define MAX_SLAVES 8
#define MAC_ADDR_LEN 6

typedef union {
	uint8_t raw[MAC_ADDR_LEN];
	uint64_t key64;
} MacAddr;

typedef struct endpoint {
	MacAddr nics[MAX_SLAVES];
	uint64_t ctr;
	uint8_t num_nics;
} endpoint;

endpoint *lookup(MacAddr key_mac);

extern endpoint localEndpoint;

#endif
