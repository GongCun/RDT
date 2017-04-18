#include "rdt.h"

int linktype;
static void callback(u_char *user, const struct pcap_pkthdr *header, const u_char *packet);

void from_net(void)
{
        pcap_t *pt;
        struct bpf_program bp;

        pt = open_pcap(dev, PCAP_TIMEOUT, FILTER, &linktype, &bp);

        /* Due to signal process, don't use loop_pcap().
         * Any exception is deal with RDT mechanism.
         */
        pcap_loop(pt, -1, callback, NULL);

        return;
}

static void callback(u_char *user, const struct pcap_pkthdr *header, const u_char *packet)
{
        int i;
        const struct ip *ip;
        int size_eth;
        int size_ip;



        /* DLT_NULL of loopback which head is 4-byte
         * DLT_EN10MB of IEEE 802.3 Ethernet
         * (10Mb, 100Mb, 1000Mb, and up) which head is 14-byte
         */
        if (linktype != 0 && linktype != 1)
                return;
        size_eth = (linktype == 0) ? 4 : 14;

        ip = (struct ip *)(packet + size_eth);
        if ((size_ip = ip->ip_hl * 4) < 20)
                return;

        const struct rdthdr *rdthdr;
        rdthdr = (struct rdthdr *)(packet + size_eth + size_ip);

        fprintf(stderr, "capture pkt: %s.%d -> ",
                        inet_ntoa(ip->ip_src), rdthdr->rdt_scid);
        fprintf(stderr, "%s.%d \n", inet_ntoa(ip->ip_dst),
                        rdthdr->rdt_dcid);
        fprintf(stderr, "capture RDT:\n");
        pkt_debug(rdthdr);

        /* Delive data to the user process */
        for (i = 0; i < MAX_CONN; i++) {

                fprintf(stderr, "from_net() i = %d\n", i);

	        switch (conn[i].cstate) {
	        case CLOSED:
                        fprintf(stderr, "state: CLOSED\n");
                        break;
	        case LISTEN:
                        fprintf(stderr, "state: LISTEN\n");
                        break;
	        case WAITING:
                        fprintf(stderr, "state: WAITING\n");
                        break;
	        case ESTABLISHED:
                        fprintf(stderr, "state: ESTABLISHED\n");
                        break;
                default:
                        fprintf(stderr, "state: Unknown\n");
                }

                if (pkt_arrive(&conn[i], packet + size_eth, header->caplen - size_eth))
                        break;
        }

        if (i >= MAX_CONN) {
                fprintf(stderr, "can't delivery the following packet: \n");
                pkt_debug((struct rdthdr *)(packet + size_eth+size_ip));
        }
}
