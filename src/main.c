#include "balancer.h"
#include "socket.h"
#include "uvif.h"
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#define IF_SLAVE 0
#define IF_VIF 1

MacAddr vifMac;

static void vif_callback(struct tpacket3_hdr *ppd) {
	struct ethhdr *eth = (struct ethhdr *)((uint8_t *)ppd + ppd->tp_mac);
	MacAddr dst;
	// PACKET FROM VIF
	dst.key64 = (uint64_t)eth->h_dest;
	endpoint *dst_endpoint = lookup(dst);
	if (likely(dst_endpoint != NULL)) {
		int remote_index = dst_endpoint->ctr++ % dst_endpoint->num_nics;
		memcpy(eth->h_dest, dst_endpoint->nics[remote_index].raw, MAC_ADDR_LEN);
		int local_index = localEndpoint.ctr++ % localEndpoint.num_nics;
		memcpy(eth->h_source, dst_endpoint->nics[local_index].raw,
			   MAC_ADDR_LEN);
	} else {
		memcpy(eth->h_source, localEndpoint.nics[0].raw, MAC_ADDR_LEN);
	}
	// TODO send to NIC
}

static void nic_callback(struct tpacket3_hdr *ppd) {
	struct ethhdr *eth = (struct ethhdr *)((uint8_t *)ppd + ppd->tp_mac);
	MacAddr src;

	// PACKET FROM NIC
	src.key64 = (uint64_t)eth->h_source;
	endpoint *src_endpoint = lookup(src);
	if (likely(src_endpoint != NULL)) {
		memcpy(eth->h_source, src_endpoint->nics[0].raw, MAC_ADDR_LEN);
		memcpy(eth->h_dest, vifMac.raw, MAC_ADDR_LEN);
	} else {
		memcpy(eth->h_dest, vifMac.raw, MAC_ADDR_LEN);
	}
	// TODO send to VIF
}

int main(int argc, char **argv) {
	char tap_name[IFNAMSIZ] = {0};
	int tap_fd = tap_alloc(tap_name);
	if (tap_fd < 0) {
		return EXIT_FAILURE;
	}
	if (argc == 1) {
		return EXIT_FAILURE;
	}
	int numsockets = argc - 1 + 1;
	struct packet_socket sockets[numsockets];
	for (int i = 1; i < numsockets - 1; i++) {
		sockets[i] = setup_interface(argv[i], nic_callback);
	}
	sockets[numsockets] = setup_interface(tap_name, vif_callback);

	start_listen(sockets, numsockets);

	return 0;
}
