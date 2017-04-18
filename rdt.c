#include "rdt.h"
#include "rtt.h"

/*
             -+- Data Transfer Path -+-

                 Pass user info by
   +---------+   UX Domain Socket          +-------+
   | User    |---------------------------> |RDT    |
   | Process |                             |Process|
   +---------+                             +-------+
       | ^                                     /\
       | |                              SIGIO /  \
       | |                                   /    \
       | |                                  v      v
       | |                         +--------+      +--------+
       | |                         |sig_io()|     /|Packet  |
       | |                         |--------|    / |Capture |
       | |                              |       /  +--------+
       | |               Pass conn info |      /         ^
       | |               by FIFO        |     / Pass pkt | Capture
       | |                              v    v  by FIFO  | pkt
       | | Recv pkt from FIFO      +--------+            |
       | +------------------------ |FIFO    |      +--------+
       +-------------------------> |RawSock |----->|external|
           Send pkt by Raw Sock    +--------+ Send +--------+
                                              pkt

*/

char dev[IFNAMSIZ];
int mtu;
int fd;
static void sig_io(int);

int main(int argc, char *argv[])
{
        const int on = 1;

        if (argc != 2)
                err_quit("%s <dev>", basename(argv[0]));
        strcpy(dev, argv[1]);

        fd = ux_serv(RDT_UX_SOCK);
	if (signal(SIGIO, sig_io) == SIG_ERR)
		err_sys("signal() of SIGIO error");
	if (fcntl(fd, F_SETOWN, getpid()) < 0)
		err_sys("fcntl() of F_SETOWN error");
	if (ioctl(fd, FIOASYNC, &on) < 0)
		err_sys("ioctl() of FIOASYNC error");
	if (ioctl(fd, FIONBIO, &on) < 0)
		err_sys("ioctl() of FIONBIO error");

        from_net();

        return(0);
}

static void sig_io(int signo)
{
        int i, n;
        struct conn_info conn_info;
        struct conn_info *ciptr = &conn_info;

#if defined (_AIX) || defined (_AIX64)
	if (signal(SIGIO, sig_io) == SIG_ERR)
		err_sys("signal() of SIGIO error");
#endif

	fprintf(stderr, "caught SIGIO\n");

        /* TO be added rexmt mechanism */
        n = sizeof(struct conn_info);
        if (recvfrom(fd, ciptr, n, 0, NULL, 0) != n)
        {
                err_sys("recvfrom() error");
        }

        conn_info_debug(ciptr);

        switch (ciptr->cact) {
                case ACTIVE:
                        i = krdt_connect(ciptr->dst, ciptr->scid, ciptr->dcid, ciptr->pid);
                        break;
                case PASSIVE:
                        i = krdt_listen(ciptr->src, ciptr->scid, ciptr->pid);
                        break;
                default:
                        err_quit("unknown connection type: %d", ciptr->cact);
        }

        fprintf(stderr, "Action: %d, idx = %d\n", ciptr->cact, i);

        return;
}
