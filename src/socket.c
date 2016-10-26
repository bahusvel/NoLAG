#define _GNU_SOURCE

#include "socket.h"
#include <arpa/inet.h>
#include <assert.h>
#include <inttypes.h>
#include <linux/ip.h>
#include <net/if.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>

static unsigned long packets_total = 0, bytes_total = 0;
static sig_atomic_t sigint = 0;

static int setup_socket(struct ring *ring, char *netdev) {
	int err, i, fd, v = TPACKET_V3;
	struct sockaddr_ll ll;
	unsigned int blocksiz = 1 << 22, framesiz = 1 << 11;
	unsigned int blocknum = 64;

	fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd < 0) {
		perror("socket");
		exit(1);
	}

	err = setsockopt(fd, SOL_PACKET, PACKET_VERSION, &v, sizeof(v));
	if (err < 0) {
		perror("setsockopt");
		exit(1);
	}

	memset(&ring->req, 0, sizeof(ring->req));
	ring->req.tp_block_size = blocksiz;
	ring->req.tp_frame_size = framesiz;
	ring->req.tp_block_nr = blocknum;
	ring->req.tp_frame_nr = (blocksiz * blocknum) / framesiz;
	ring->req.tp_retire_blk_tov = 60;
	ring->req.tp_feature_req_word = TP_FT_REQ_FILL_RXHASH;

	err = setsockopt(fd, SOL_PACKET, PACKET_RX_RING, &ring->req,
					 sizeof(ring->req));
	if (err < 0) {
		perror("setsockopt");
		exit(1);
	}

	ring->map = mmap(NULL, ring->req.tp_block_size * ring->req.tp_block_nr,
					 PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);
	if (ring->map == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	ring->rd = malloc(ring->req.tp_block_nr * sizeof(*ring->rd));
	assert(ring->rd);
	for (i = 0; i < ring->req.tp_block_nr; ++i) {
		ring->rd[i].iov_base = ring->map + (i * ring->req.tp_block_size);
		ring->rd[i].iov_len = ring->req.tp_block_size;
	}

	memset(&ll, 0, sizeof(ll));
	ll.sll_family = PF_PACKET;
	ll.sll_protocol = htons(ETH_P_ALL);
	ll.sll_ifindex = if_nametoindex(netdev);
	ll.sll_hatype = 0;
	ll.sll_pkttype = 0;
	ll.sll_halen = 0;

	err = bind(fd, (struct sockaddr *)&ll, sizeof(ll));
	if (err < 0) {
		perror("bind");
		exit(1);
	}

	return fd;
}

static void walk_block(struct block_desc *pbd, const int block_num,
					   packet_callback callback) {
	int num_pkts = pbd->h1.num_pkts, i;
	unsigned long bytes = 0;
	struct tpacket3_hdr *ppd;

	ppd = (struct tpacket3_hdr *)((uint8_t *)pbd + pbd->h1.offset_to_first_pkt);
	for (i = 0; i < num_pkts; ++i) {
		bytes += ppd->tp_snaplen;

		callback(ppd);

		ppd = (struct tpacket3_hdr *)((uint8_t *)ppd + ppd->tp_next_offset);
	}

	packets_total += num_pkts;
	bytes_total += bytes;
}

static void flush_block(struct block_desc *pbd) {
	pbd->h1.block_status = TP_STATUS_KERNEL;
}

struct packet_socket setup_interface(char *ifname, packet_callback callback) {
	struct packet_socket socket = {0};
	socket.callback = callback;

	socket.fd = setup_socket(&socket.ring, ifname);
	assert(socket.fd > 0);

	socket.pfd.fd = socket.fd;
	socket.pfd.events = POLLIN | POLLERR;
	socket.pfd.revents = 0;

	return socket;
}

static void sighandler(int num) { sigint = 1; }

void start_listen(struct packet_socket *sockets, int num_sockets) {
	signal(SIGINT, sighandler);
	struct block_desc *pbd;
	unsigned int blocks = 64;
	unsigned int *blocknum;
	struct pollfd pfds[num_sockets];
	for (int i = 0; i < num_sockets; i++) {
		pfds[i] = sockets[i].pfd;
	}
	while (likely(!sigint)) {
		for (int i = 0; i < num_sockets; i++) {
			blocknum = &sockets[i].blocknum;
			pbd = (struct block_desc *)sockets[i].ring.rd[*blocknum].iov_base;
			if ((pbd->h1.block_status & TP_STATUS_USER) != 0) {
				walk_block(pbd, *blocknum, sockets[i].callback);
				flush_block(pbd);
				*blocknum = (*blocknum + 1) % blocks;
			}
		}
		poll(pfds, num_sockets, -1);
	}
}
