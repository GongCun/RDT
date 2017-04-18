#include "rdt.h"


ssize_t make_pkt(struct in_addr src, struct in_addr dst, int scid, int dcid, uint32_t seq, uint8_t  flags, void *data, size_t nbyte, void *buf)
{
        struct ip *ip;
        struct rdthdr *rdthdr;

        bzero(buf, IP_LEN + RDT_LEN + nbyte);

        /* Fill the IP header
         */
        ip = (struct ip *)buf;
        ip->ip_hl = 5;
        ip->ip_v = 4;
        ip->ip_tos = 0;
#if defined(_LINUX) || defined(_OPENBSD)
        ip->ip_len = htons(IP_LEN + RDT_LEN + nbyte);
#else
        ip->ip_len = IP_LEN + RDT_LEN + nbyte;
#endif
        ip->ip_id = htons(0);
#ifdef _LINUX
        ip->ip_off = htons(IP_DF);
#else
        ip->ip_off = IP_DF;
#endif
        ip->ip_ttl = 64;
        ip->ip_p = IPPROTO_RDT;
        memcpy(&ip->ip_src, &src, sizeof(src));
        memcpy(&ip->ip_dst, &dst, sizeof(dst));
        if (do_checksum((u_char *)buf, IPPROTO_IP, IP_LEN) < 0)
                err_quit("do_checksum() of IP error");

        /* Fill the RDT header
         */
        rdthdr = (struct rdthdr *)(buf + IP_LEN);
        rdthdr->rdt_scid = scid;
        rdthdr->rdt_dcid = dcid;
        rdthdr->rdt_seq = seq;
        rdthdr->rdt_flags = flags;
        rdthdr->rdt_len = htons(RDT_LEN + nbyte);
        rdthdr->rdt_sum = 0; /* calculater later */

        /* Fill the Data
         */
        memcpy((u_char *)(buf + IP_LEN + RDT_LEN), (u_char *)data, nbyte);

        /* Now do the checksum */
        if (do_checksum((u_char *)buf, IPPROTO_RDT, RDT_LEN + nbyte) < 0)
                err_quit("do_checksum() of RDT error");

        return(IP_LEN + RDT_LEN + nbyte);
}


