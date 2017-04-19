#include "rdt.h"

void 
pkt_debug(const struct rdthdr *rdthdr)
{
	fprintf(stderr, "%d %d %lu 0x%02x %d 0x%x 0x%02x\n",
		rdthdr->rdt_scid,
		rdthdr->rdt_dcid,
		(unsigned long)rdthdr->rdt_seq,
		rdthdr->rdt_flags,
		ntohs(rdthdr->rdt_len),
                rdthdr->rdt_sum,
		rdthdr->rdt_pads);
	fflush(stderr);
}
