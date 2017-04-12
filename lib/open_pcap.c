#include "tcpi.h"

#define CAPLEN 1514

pcap_t *open_pcap(const char *device, int to_ms, char *cmd, int *linktype, struct bpf_program *bp)
{
        char errbuf[PCAP_ERRBUF_SIZE];
        pcap_t *pt;
        bpf_u_int32 net, mask;

        bzero(errbuf, sizeof(errbuf));

        assert(device != NULL);

        if ((pt = pcap_open_live(device, CAPLEN, 1, to_ms, errbuf)) == NULL)
                err_quit("pcap_open_live: %s", errbuf);

        if (strlen(errbuf) != 0)
                fprintf(stderr, "pcap_open_live warning: %s\n", errbuf);

        bzero(errbuf, sizeof(errbuf));
        if (pcap_lookupnet(device, &net, &mask, errbuf) < 0)
                err_quit("pcap_lookupnet: %s", errbuf);
        if (pcap_compile(pt, bp, cmd, 0, mask) < 0)
                err_quit("pcap_compile: %s", pcap_geterr(pt));
        if (pcap_setfilter(pt, bp) < 0)
                err_quit("pcap_setfilter: %s", pcap_geterr(pt));
        if ((*linktype = pcap_datalink(pt)) < 0)
                err_quit("pcap_datalink: %s", pcap_geterr(pt));

        return pt;

}
