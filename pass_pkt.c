#include "rdt.h"

ssize_t pass_pkt(int fd, struct conn_info *ciptr, u_char *buf, ssize_t buflen)
{
        ssize_t n;
        struct iovec iov[2];
        iov[0].iov_base = ciptr;
        iov[0].iov_len = sizeof(struct conn_info);
        iov[1].iov_base = buf;
        iov[1].iov_len = buflen;

        if ((n = writev(fd, iov, 2)) != (sizeof(struct conn_info) + buflen))
                err_sys("writev() error");
        return(n);
}
