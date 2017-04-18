#include "rdt.h"

/* Receive network frame from below */
ssize_t rdt_recv(void *buf, size_t nbyte)
{
	int ret, n, ack;
	struct rdthdr *rptr;
	struct conn_user *cptr;

	cptr = conn_user;
	ack = cptr->ack;

	n = read(cptr->rcvfd, cptr->rcvpkt, cptr->mss);
	rptr = (struct rdthdr *)cptr->rcvpkt;

        fprintf(stderr, "> rdt_recv() expect seq: %d\n", ack);
        fprintf(stderr, "> recv pkt:\n");
        pkt_debug(rptr);

	while((n < sizeof(struct rdthdr)) || ((rptr->rdt_seq) != ack) ||
			(!chk_chksum((uint16_t *)cptr->rcvpkt, ntohs(rptr->rdt_len))))
	{
		/* Send _NoAck_ packet, partner haven't recv
                 * our ACK.
                 * */
		n = make_pkt(cptr->src, cptr->dst, cptr->scid, cptr->dcid,
                        rptr->rdt_seq, RDT_ACK, NULL, 0, cptr->rcvpkt);
		if ((n = to_net(cptr->sfd, cptr->rcvpkt, n, cptr->dst)) < 0)
			return(n);

		/* Wait packet again */
		n = read(cptr->rcvfd, cptr->rcvpkt, cptr->mss);
		rptr = (struct rdthdr *)cptr->rcvpkt;

                fprintf(stderr, "> recv pkt:\n");
                pkt_debug(rptr);
	}

	/* Now get the correct pkt, ack the partner, and delivery 
 	 * data to user.
	 */
	n -= RDT_LEN;
	if (n > nbyte || n < 0)
		err_quit("recv %d bytes exceed the buf size %d\n", n, nbyte);

	if (n > 0) {
		memcpy(buf, (void *)cptr->rcvpkt + RDT_LEN, n);
		ret = n;
	} else {
		ret = 0;
	}


	n = make_pkt(cptr->src, cptr->dst, cptr->scid, cptr->dcid,
			ack, RDT_ACK, NULL, 0, cptr->rcvpkt);
	cptr->ack = (cptr->ack + 1) % 2;

	if ((n = to_net(cptr->sfd, cptr->rcvpkt, n, cptr->dst)) < 0)
		return(n);

        if (ret == 0) {
                /* Send Fin and wait Ack */
	        if ((n = rexmt_pkt(cptr, RDT_FIN, NULL, 0)) < 0)
	        	err_sys("rexmt_pkt() error");
        }

	return(ret);

}


