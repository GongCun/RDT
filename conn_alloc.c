#include "rdt.h"
#include <sys/mman.h>


void conn_alloc(void)
{
        if (conn_user != NULL)
                return;
#ifdef MAP_ANON
	if ((conn_user = mmap(0, sizeof(struct conn_user), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED)
        {
		err_sys("mmap() error");
        }
#else
        int fd;
        if ((fd = open("/dev/zero", O_RDWR, 0)) < 0)
                err_sys("open() /dev/zero error");
        if ((conn_user = mmap(0, sizeof(struct conn_user), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
        {
                err_sys("mmap() error");
        }
#endif
        return;
}


