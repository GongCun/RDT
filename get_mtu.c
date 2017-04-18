#include "rdt.h"


int get_mtu(char *dev)
{
        int sockfd;
        struct ifreq ifr;
#if defined (SIOCGIFMTU) && defined (HAVE_STRUCT_IFREQ_IFR_MTU)
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
                err_sys("socket() error");
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
        if (ioctl(sockfd, SIOCGIFMTU, &ifr) < 0)
                err_sys("ioctl error");
        return((int)ifr.ifr_mtu);
#else
        return(GUESS_MTU);
#endif
}
