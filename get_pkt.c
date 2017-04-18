#include "rdt.h"

ssize_t get_pkt(int fd, struct conn_info *ciptr, u_char *buf, ssize_t buflen)
{
        ssize_t n;
        struct iovec iov[2];
        iov[0].iov_base = ciptr;
        iov[0].iov_len = sizeof(struct conn_info);
        iov[1].iov_base = buf;
        iov[1].iov_len = buflen;

        if ((n = readv(fd, iov, 2)) < sizeof(struct conn_info))
                err_sys("readv() %d bytes error", n);
        return(n);
}
