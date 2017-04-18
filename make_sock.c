#include "rdt.h"

int 
make_sock(void)
{
	int fd;
	const int on = 1;

	if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		err_sys("socket() error");
	if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
		err_sys("setsockopt() of IP_HDRINCL error");
	return (fd);
}
