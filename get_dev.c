#include "rdt.h"

int get_dev(struct in_addr addr, char *dev)
{
        int exist = 0;
        struct sockaddr_in *sa;
#if defined (HAVE_GETIFADDRS) && defined (HAVE_IFADDRS_STRUCT)
	struct ifaddrs *ifap, *ifa;

	if (getifaddrs(&ifap) < 0)
		err_sys("getifaddrs() error");
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family != AF_INET || (ifa->ifa_flags & IFF_UP) == 0)
			continue;
		sa = (struct sockaddr_in *)ifa->ifa_addr;
		if (sa->sin_addr.s_addr != addr.s_addr)
			continue;
		strncpy(dev, ifa->ifa_name, IFNAMSIZ);
                exist = 1;
		break;
	}
	freeifaddrs(ifap);
#else				/* Don't have getifaddrs() */
	struct ifi_info *ifi, *ifihead;
	for (ifihead = ifi = Get_ifi_info(); ifi; ifi = ifi->ifi_next) {
		sa = (struct sockaddr_in *)ifi->ifi_addr;
		if (sa->sin_addr.s_addr != addr.s_addr)
			continue;
		strncpy(dev, ifi->ifi_name, IFNAMSIZ);
		exist = 1;
		break;
	}
	free_ifi_info(ifihead);
#endif				/* HAVE_GETIFADDRS */

        return(exist);
}
