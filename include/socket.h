#ifndef __RSOCKET__
#define __RSOCKET__

#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <poll.h>
#include <stdint.h>

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

typedef void (*packet_callback)(struct tpacket3_hdr *ppd);

struct block_desc {
	uint32_t version;
	uint32_t offset_to_priv;
	struct tpacket_hdr_v1 h1;
};

struct ring {
	struct iovec *rd;
	uint8_t *map;
	struct tpacket_req3 req;
};

struct packet_socket {
	int fd;
	packet_callback callback;
	unsigned int blocknum;
	struct ring ring;
	struct pollfd pfd;
};

void start_listen(struct packet_socket *sockets, int num_sockets);
struct packet_socket setup_interface(char *ifname, packet_callback callback);

#endif
