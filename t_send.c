#include "rdt.h"

char dev[IFNAMSIZ];
int mtu;

int main(int argc, char *argv[])
{
        int n;
	int fd[2];
        struct in_addr dst;
        char buf[MAXLINE]; /* the MAXLINE shouldn't exceed
                              the MTU - IP_LEN -RDT_LEN */

        if (argc != 3)
                err_quit("usage: %s <IPaddress> <#CID>", basename(argv[0]));
        if (inet_aton(argv[1], &dst) != 1) {
                errno = EINVAL;
                err_sys("inet_aton() error");
        }

        rdt_connect(dst, -1, atoi(argv[2]));
	rdt_pipe(fd);

	while ((n = read(0, buf, MAXLINE)) > 0) {
again:
		if (write(fd[1], buf, n) != n) {
                        if (errno == EWOULDBLOCK || errno == EAGAIN)
                                goto again;
                        else
                                err_sys("write() error");
                }
		if ((n = read(fd[0], buf, MAXLINE)) > 0)
			write(1, buf, n);
	}

        return(0);
}
