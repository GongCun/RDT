#include "rdt.h"

/* Receive network frame from below */
ssize_t rdt_recv(void *buf, size_t nbyte)
{
	int ret, n, ack;
	struct rdthdr *rptr;
	struct conn_user *cptr;
	static struct timeval *tv = NULL;
	struct timeval curr;

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
		/* Send the most recently received in-order packet */
		n = make_pkt(cptr->src, cptr->dst, cptr->scid, cptr->dcid,
			     ack - 1 , RDT_ACK, NULL, 0, cptr->rcvpkt);
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

	/*
	 * Use cumulative acknowledgment, but don't use the timer,
	 * because:
	 *
	 * 1) I don't know how to use the synchronization mechanism to
	 *    keep the reply ACK right.
	 *
	 * 2) It would be dangerous if the signal SIGALRM was called
	 *    repeatedly.
	 */

	if (tv == NULL && (tv = malloc(sizeof(struct timeval))) == NULL)
		err_sys("malloc() tv error");

	if (gettimeofday(&curr, (struct timezone *)NULL) < 0)
		err_sys("gettimeofday() error");

	if (((curr.tv_sec - tv->tv_sec) * 1000.0 +
	     (curr.tv_usec - tv->tv_usec) / 1000.0) >= 200.0 || ret == 0) {
	
		n = make_pkt(cptr->src, cptr->dst, cptr->scid, cptr->dcid,
			     ack, RDT_ACK, NULL, 0, cptr->rcvpkt);

		if ((n = to_net(cptr->sfd, cptr->rcvpkt, n, cptr->dst)) < 0)
			return(n);

		tv->tv_sec = curr.tv_sec;
		tv->tv_usec = curr.tv_usec;
	}

	/*
	 * fprintf(stderr, "rdt_recv(): Ack to_net():\n");
	 * pkt_debug((struct rdthdr *)(cptr->rcvpkt + IP_LEN));
	 */

	cptr->ack++;

	if (ret == 0) {
		close(readin); /* notify send process do rdt_fin() */
	}

	return(ret);

}


