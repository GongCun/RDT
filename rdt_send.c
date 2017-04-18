#include "rdt.h"
#include "rtt.h"


static int pipefd;
static void sigalrm(int);

/* FSM Sender FSM, wait three type of event:
 * 1) Data from above (fd)
 * 2) Ack pkt from network (conn_user->sndfd)
 * 3) Timeout (pipefd)
 */
int rdt_send(int fd)
{
	int i, n, len, maxfd;
        int pfd[2];
        char c;
	struct conn_user *cptr;
        fd_set rset;
        uint32_t base, nextseq;
        struct rtt_info *rptr;
        struct rdthdr *rdthdr;

        readin = fd;

        if (pipe(pfd) < 0)
                err_sys("pipe() error");
        pipefd = pfd[1];

        if (signal(SIGALRM, sigalrm) == SIG_ERR)
                err_sys("signal() SIGALRM error");

	cptr = conn_user;
        rptr = &rtt_info;
        FD_ZERO(&rset);
        base = 0;
        cptr->seq = nextseq = 0;
        rtt_init(rptr);
        rtt_newpack(rptr);

        for (;;) {
                FD_SET(fd, &rset);
                FD_SET(cptr->sndfd, &rset);
                FD_SET(pfd[0], &rset);
                maxfd = max(cptr->sndfd, pfd[0]);
                maxfd = max(maxfd, fd);

again:
                if (select(maxfd+1, &rset, NULL, NULL, NULL) < 0) {
                        if (errno != EINTR)
                                err_sys("select() error");
                        else
                                goto again;
                }

                if (FD_ISSET(fd, &rset)) {
                        if (nextseq < base + WINSIZE) {
                                n = read(fd, cptr->sndpkt, cptr->mss - IP_LEN - RDT_LEN);
                                fprintf(stderr, "rdt_send() read from user %d bytes\n", n);
                                if (n <= 0) {
                                        FD_CLR(fd, &rset);
                                        rdt_fin();
                                        return(0);
                                } else {
					i = nextseq % WINSIZE;
					len = make_pkt(cptr->src, cptr->dst, cptr->scid, cptr->dcid,
						       nextseq, RDT_DATA, cptr->sndpkt, n, cptr->snddat[i]);
                                        n = to_net(cptr->sfd, cptr->snddat[i], len, cptr->dst);
					fprintf(stderr, "rdt_send() send %d bytes data pkt of idx %d\n",
						n, i);
					pkt_debug((struct rdthdr *)(cptr->snddat[i] + IP_LEN));
					if (nextseq == base)
						alarm(rtt_start(rptr));
					nextseq++;
                                        cptr->seq = nextseq;
				}
                        }
                } /* end of call from above event */

                if (FD_ISSET(cptr->sndfd, &rset)) { /* recv Ack */
                        n = read(cptr->sndfd, cptr->sndpkt, cptr->mss);
                        rdthdr = (struct rdthdr *)cptr->sndpkt;
                        fprintf(stderr, "> rdt_send() recv Ack pkt:\n");
                        pkt_debug((struct rdthdr *)cptr->sndpkt);
                        base = rdthdr->rdt_seq + 1;
			fprintf(stderr, "base = %d, nextseq = %d\n", base, nextseq);
                        if (base == nextseq) {
                                rtt_stop(rptr);
                                alarm(0);
                        }
                        else
                                alarm(rtt_start(rptr));
                }

                if (FD_ISSET(pfd[0], &rset)) { /* timed out */
                        read(pfd[0], &c, 1);
                        if (rtt_timeout(rptr) < 0) {
                                errno = ETIMEDOUT;
                                return(-1);
                        }
                        fprintf(stderr, "timed out, restart timer:\n");
                        rtt_debug(rptr);
                        alarm(rtt_start(rptr));

                        /* re-transmit un-Ack pkt */
			fprintf(stderr, "base = %d, nexseq = %d\n", base, nextseq);
                        for (i = base; i < nextseq; i++) {
                                rdthdr = (struct rdthdr *)(cptr->snddat[i%WINSIZE] + IP_LEN);
                                len = IP_LEN + ntohs(rdthdr->rdt_len);
                                fprintf(stderr, "idx = %d, rexmit %d bytes\n", i, len);
                                n = to_net(cptr->sfd, cptr->snddat[i%WINSIZE], len, cptr->dst);
                                fprintf(stderr, "rdt_send(): to_net() %d bytes\n", n);
                        }
                }
        }

        return(0);
}

static void sigalrm(int signo)
{
        char c;
        write(pipefd, (void *)&c, 1);
}

