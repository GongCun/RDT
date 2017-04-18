#include "rdt.h"

static void sig_hand(int signo)
{
        if (signo == SIGINT || signo == SIGHUP || signo == SIGQUIT)
                exit(1);
}


int rdt_connect(struct in_addr dst, int scid, int dcid)
{
        int i, n, fd;
        pid_t pid;
        struct sockaddr_un un;
        struct in_addr src;
        struct conn_info conn_info;

        if (signal(SIGINT, sig_hand) == SIG_ERR ||
            signal(SIGHUP, sig_hand) == SIG_ERR ||
            signal(SIGQUIT, sig_hand) == SIG_ERR)
        {
                err_sys("signal() error");
        }

        src = get_addr(dst);
        pid = getpid();

        /* allocate share area for send
         * and recv process.
         */
        conn_alloc(); 
        conn_info.cact = ACTIVE;
        conn_info.pid = pid;
        conn_info.src = conn_user->src = src;
        conn_info.dst = conn_user->dst = dst;
        conn_info.scid = conn_user->scid = scid;
        conn_info.dcid = conn_user->dcid = dcid;
        conn_user->sndfd = make_fifo(pid, "snd");
        conn_user->rcvfd = make_fifo(pid, "rcv");
        conn_user->sfd = make_sock();
	conn_user->seq = conn_user->ack = 0;

        if (!mtu) {
                if (dev[0] == 0 && !get_dev(src, dev))
                        err_quit("can't get dev name");
                mtu = get_mtu(dev);
        }
        n = min(mtu, 1500);
        conn_user->mss = n;
        if ((conn_user->sndpkt = malloc(n)) == NULL)
                err_sys("malloc() sndpkt error");
        if ((conn_user->rcvpkt = malloc(n)) == NULL)
                err_sys("malloc() rcvpkt error");
        for (i = 0; i < WINSIZE; i++) {
                if ((conn_user->snddat[i] = malloc(n)) == NULL)
                        err_sys("malloc() snddat[%d] error", i);
        }

        if ((fd = ux_cli(RDT_UX_SOCK, &un)) < 0)
                err_sys("ux_cli() error");
        n = sizeof(struct conn_info);
        if (sendto(fd, &conn_info, n, 0, (struct sockaddr *)&un,
                                sizeof(struct sockaddr_un)) != n)
        {
                err_sys("sendto() error");
        }
	get_pkt(conn_user->sndfd, &conn_info, NULL, 0);
	conn_user->scid = conn_info.scid;

        if (rexmt_pkt(conn_user, RDT_REQ, NULL, 0) < 0)
                err_sys("rexmt_pkt() error");

        fprintf(stderr, "rdt_connect() succeed\n");
        pkt_debug((struct rdthdr *)conn_user->sndpkt);
        
        return(0);

}
