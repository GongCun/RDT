#include "rdt.h"

void 
pkt_debug(const struct rdthdr *rdthdr)
{
	fprintf(stderr, "%d %d %02x 0x%02x %d 0x%x\n",
		rdthdr->rdt_scid,
		rdthdr->rdt_dcid,
		rdthdr->rdt_seq,
		rdthdr->rdt_flags,
		ntohs(rdthdr->rdt_len),
                rdthdr->rdt_sum);
	fflush(stderr);
}
