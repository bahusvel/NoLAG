#include "balancer.h"
#include "list.h"
#include <stdlib.h>

typedef struct mac_map_entry {
	uint64_t key_mac;
	endpoint *endpoint;
	struct mac_map_entry *next;
} mac_map_entry;

struct __advertise_frame {
	uint64_t master_mac;
	MacAddr slave_macs[MAX_SLAVES];
};

typedef union {
	struct __advertise_frame frame;
	unsigned char data[sizeof(struct __advertise_frame)];
} advertise_frame;

static mac_map_entry mac_map;

endpoint *lookup(MacAddr key_mac) {
	mac_map_entry *iter_ptr;
	LIST_FOREACH(iter_ptr, mac_map) {
		if (iter_ptr->key_mac == key_mac.key64)
			return iter_ptr->endpoint;
	}
	return NULL;
}
