#include "list.h"
#include <inttypes.h>
#include <stdlib.h>

#define MAX_SLAVES 8

typedef struct mac_map_entry {
	uint64_t key_mac;
	uint64_t master_mac;
	struct mac_map_entry *next;
} mac_map_entry;

struct __advertise_frame {
	uint64_t master_mac;
	uint64_t slave_macs[MAX_SLAVES];
};

typedef union {
	struct __advertise_frame frame;
	unsigned char data[sizeof(struct __advertise_frame)];
} advertise_frame;

static mac_map_entry mac_map;

uint64_t is_present(int64_t key_mac) {
	mac_map_entry *iter_ptr;
	LIST_FOREACH(iter_ptr, mac_map) {
		if (iter_ptr->key_mac == key_mac)
			return iter_ptr->master_mac;
	}
	return 0;
}
