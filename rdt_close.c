#include "rdt.h"

void rdt_close(void)
{
	int n;
	struct conn_user *cptr;
        struct rdthdr *rptr;
	cptr = conn_user;

	n = cptr->mss - IP_LEN - RDT_LEN;

        /* Wait Fin and send Ack */
        do {
                n = read(cptr->rcvfd, cptr->rcvpkt, cptr->mss);
		rptr = (struct rdthdr *)cptr->rcvpkt;
        } while (n < RDT_LEN || rptr->rdt_flags != RDT_FIN);

        fprintf(stderr, ">> rdt_close() recv Fin:\n");
        pkt_debug(rptr);

        n = make_pkt(cptr->src, cptr->dst, cptr->scid, cptr->dcid,
                        rptr->rdt_seq, RDT_ACK, NULL, 0, cptr->rcvpkt);
        if ((n = to_net(cptr->sfd, cptr->rcvpkt, n, cptr->dst)) < 0)
                err_sys("to_net() error");

        fprintf(stderr, ">> rdt_close() send Ack:\n");
        pkt_debug((struct rdthdr *)(cptr->rcvpkt + IP_LEN));

        /* Send Fin and wait Ack */
	if ((n = rexmt_pkt(cptr, RDT_FIN, NULL, 0)) < 0)
		err_sys("rexmt_pkt() error");

        /* Notify RDT process to cleanup */
        /* ... */

}


