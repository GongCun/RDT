#include "rdt.h"
#include "rtt.h"

static void sig_alrm(int);
static sigjmp_buf jmpbuf;

ssize_t rexmt_pkt(struct conn_user *cptr, uint8_t flag, void *buf, size_t nbyte)
{
        int n, ret, seq;
        static int init = 0;

        struct rtt_info *rptr;
	const struct rdthdr *rdthdr;

        rptr = &rtt_info;
	seq = cptr->seq;

        if (init++ == 0)
                rtt_init(rptr);

        if (signal(SIGALRM, sig_alrm) == SIG_ERR)
                err_sys("signal() of SIGALRM error");

        rtt_newpack(rptr);

rexmt:
        n = make_pkt(cptr->src, cptr->dst, cptr->scid, cptr->dcid,
                        seq, flag, buf, nbyte, cptr->sndpkt);
        fprintf(stderr, "> rexmt_pkt() send pkt:\n");
        pkt_debug((struct rdthdr *)(cptr->sndpkt + IP_LEN));

        alarm(rtt_start(rptr));
        while ((ret = to_net(cptr->sfd, cptr->sndpkt, n, cptr->dst)) < 0)
                ;

        if (sigsetjmp(jmpbuf, 1) != 0) {
                if (rtt_timeout(rptr) < 0) {
                        errno = ETIMEDOUT;
                        return(-1);
                }
                rtt_debug(rptr);
                goto rexmt;
        }

again:
        do {
                n = read(cptr->sndfd, cptr->sndpkt, cptr->mss);
		rdthdr = (struct rdthdr *)cptr->sndpkt;

        } while ((n < sizeof(struct rdthdr)) || (rdthdr->rdt_seq != seq) || 
			(!chk_chksum((uint16_t *)cptr->sndpkt, ntohs(rdthdr->rdt_len))));

        fprintf(stderr, "> rexmt_pkt() recv pkt:\n");
        pkt_debug(rdthdr);

	switch (flag) {

	case RDT_REQ:
	{
		if (rdthdr->rdt_flags != RDT_ACC)
			goto again;
		break;
	}
	case RDT_DATA:
	case RDT_FIN:
	{
		if (rdthdr->rdt_flags != RDT_ACK)
			goto again;
		break;
	}
	default:;

	}

        alarm(0);
        rtt_stop(rptr);
        rtt_debug(rptr);

	/* Return sent network frame length */
        return(ret);
}


static void sig_alrm(int signo)
{
#if defined (_AIX) || defined (_AIX64)
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)
		err_sys("signal() of SIGALRM error");
#endif
        siglongjmp(jmpbuf, 1);
}

