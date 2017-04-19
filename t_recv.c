#include "rdt.h"

char dev[IFNAMSIZ];
int mtu;

int main(int argc, char *argv[])
{
        int n;
        int scid;
	int fd[2];
        struct in_addr src;
        char buf[MAXLINE]; /* the MAXLINE shouldn't exceed
                              the MTU - IP_LEN -RDT_LEN */

        if (argc != 3)
                err_quit("usage: %s <IPaddress> <#CID>", basename(argv[0]));
        if (inet_aton(argv[1], &src) != 1) {
                errno = EINVAL;
                err_sys("inet_aton() error");
        }

        scid = atoi(argv[2]);
        rdt_listen(src, scid);
	rdt_pipe(fd);

	while ((n = read(fd[0], buf, MAXLINE)) > 0) {
                /*
                 * fprintf(stderr, "t_recv() read %d bytes:\n", n);
                 * write(2, buf, n);
                 */
again:
		if (write(fd[1], buf, n) != n) {
                       if (errno == EWOULDBLOCK || errno == EAGAIN)
                               goto again;
                       else
                               err_sys("write() error");
                }
        }

        return(0);
}

