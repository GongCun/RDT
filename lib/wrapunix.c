#include "tcpi.h"

void *Malloc(size_t size)
{
    void *ptr;

    if ((ptr = malloc(size)) == NULL)
        err_sys("malloc error");
    return ptr;
}

int Socket(int family, int type, int protocol)
{
    int n;

    if ((n = socket(family, type, protocol)) < 0)
        err_sys("socket error");
    return n;
}

int Ioctl(int fd, unsigned long request, void *arg)
{
    int n;
    if ((n = ioctl(fd, request, arg)) == -1)
        err_sys("ioctl error");
    return n;
}

void *Calloc(size_t count, size_t size)
{
    void *ptr;
    if ((ptr = calloc(count, size)) == NULL)
        err_sys("calloc error");
    return ptr;
}

