#include "rdt.h"

static void sig_alrm(int);

/* Receive network frame from below */
ssize_t rdt_recv(void *buf, size_t nbyte)
{
	int ret, n, ack;
	struct rdthdr *rptr;
	static struct conn_user *cptr;
	static struct itimerval delay;

	cptr = conn_user;
	ack = cptr->ack;


	if (signal(SIGALRM, sig_alrm) == SIG_ERR)
		err_sys("signal() error");

	/*
	 * Start Ack timer every 200 ms
	 */
	delay.it_value.tv_sec = 0;
	delay.it_value.tv_usec = 200000; /* 200 ms */
	delay.it_interval.tv_sec = 0;
	delay.it_interval.tv_usec = 200000; /* 200 ms */

	if (setitimer(ITIMER_REAL, &delay, NULL) < 0)
		err_sys("settimer() error");
	

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

	/*
	 * Now get the correct pkt, need to use a separate variable to save
	 * ack to distinguish between the already confirmed ack or the
	 * expected sequence, use the ACK timer to send the cumulative ack.
	 */

	cptr->cumack = ack;
	n -= RDT_LEN;
	if (n > nbyte || n < 0)
		err_quit("recv %d bytes exceed the buf size %d\n", n, nbyte);

	if (n > 0) {
		memcpy(buf, (void *)cptr->rcvpkt + RDT_LEN, n); /* copy the data to user */
		ret = n;
	} else {
		ret = 0;
	}


	cptr->ack++;

	if (ret == 0) {
		sig_alrm(SIGALRM);
		close(readin); /* notify send process do rdt_fin() */
	}

	return(ret);

}

static void sig_alrm(int signo)
{
	
	static unsigned char *buf = NULL;
	static uint32_t sent = -1;
	ssize_t n;

	if (conn_user == NULL || conn_user->cumack == sent)
		return;
	
	if (buf == NULL)
		buf = Malloc(conn_user->mss);
	
	n = make_pkt(conn_user->src, conn_user->dst, conn_user->scid, conn_user->dcid,
		     conn_user->cumack, RDT_ACK, NULL, 0, buf);

	if ((n = to_net(conn_user->sfd, buf, n, conn_user->dst)) < 0)
		err_sys("to_net() error");

	sent = conn_user->cumack;

	return;
}

