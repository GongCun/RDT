#include "tcpi.h"

struct ifi_info *get_ifi_info(void)
{
    struct ifi_info *pifi, *pifihead, **ppifinext;
    struct ifconf ifc;
    struct ifreq *ifr, ifrcopy;
    int sockfd, len, lastlen;
    char *buf, *cptr;

    sockfd = xsocket(AF_INET, SOCK_DGRAM, 0);
    lastlen = 0;
    len = 100 * sizeof(struct ifreq); /* initial buffer size guess */
    for (;;) {
        buf = xmalloc(len);
        ifc.ifc_len = len;
        ifc.ifc_buf = buf;

        /* 
         * UNPv1_3thEd: Chapter13, Page 563.
         * If an error of EINVAL is returned by ioctl,
         * and we have not yet had a successful return
         * (i.e., lastlen is still 0), we have not yet allocated
         * a buffer large enough and continue through the loop.
         */
        if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
            if (errno != EINVAL || lastlen != 0)
                err_sys("ioctl error");
        } else {
            if (ifc.ifc_len == lastlen)
                break;
            lastlen = ifc.ifc_len;
        }
        len += 10 * sizeof(struct ifreq);
        free(buf);
    }

    char *ptr, *sdlname, *haddr;
    int idx = 0, hlen = 0;

    pifihead = NULL;
    ppifinext = &pifihead;
    sdlname = NULL;

    struct sockaddr_in *sinptr;

    for (ptr = buf; ptr < buf + ifc.ifc_len; ) {
        ifr = (struct ifreq *)ptr;
#ifdef HAVE_SOCKADDR_SA_LEN
        len = max(sizeof(struct sockaddr), ifr->ifr_addr.sa_len);
#else
        switch (ifr->ifr_addr.sa_family) {
#ifdef IPV6
            case AF_INET6:
                len = sizeof(struct sockaddr_in6);
                break;
#endif
            case AF_INET:
            default:
                len = sizeof(struct sockaddr);
                break;
        }
#endif /* HAVE_SOCKADDR_SA_LEN */
        ptr += sizeof(ifr->ifr_name) + len; /* for next one in buffer */
#ifdef IPV6
        if (ifr->ifr_addr.sa_family == AF_INET6)
            continue;
#endif /* Don't handle IPv6 */

#ifdef HAVE_SOCKADDR_DL_STRUCT
        /*
         * If the system is one that returns AF_LINK sockaddrs in SIOCGIFCONF,
         * copy the interface index and the hardware address information from
         * the AF_LINK sockaddr.
         */
        if (ifr->ifr_addr.sa_family == AF_LINK) {
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)&ifr->ifr_addr;
            sdlname = ifr->ifr_name;

            idx = sdl->sdl_index;
            haddr = sdl->sdl_data + sdl->sdl_nlen;
            hlen = sdl->sdl_alen;
        }
#endif /* HAVE_SOCKADDR_DL_STRUCT */

        if (ifr->ifr_addr.sa_family != AF_INET)
            continue; /* ignore if not IPv4 */

        if ((cptr = strchr(ifr->ifr_name, ':')) != NULL)
            *cptr = 0; /* replace colon with null */

        ifrcopy = *ifr;

        int flags;
        xioctl(sockfd, SIOCGIFFLAGS, &ifrcopy);
        flags = ifrcopy.ifr_flags;
        if ((flags & IFF_UP) == 0)
            continue; /* ignore if interface not up */

        pifi = xcalloc(1, sizeof(struct ifi_info));
        *ppifinext = pifi;
        ppifinext = &pifi->ifi_next; /* pointer to next one
                                       goes here */
        pifi->ifi_flags = flags;
#if defined(SIOCGIFMTU) && defined(HAVE_STRUCT_IFREQ_IFR_MTU)
        xioctl(sockfd, SIOCGIFMTU, &ifrcopy);
        pifi->ifi_mtu = ifrcopy.ifr_mtu;
#else
        pifi->ifi_mtu = 0;
#endif
        memcpy(pifi->ifi_name, ifr->ifr_name, IFI_NAME);
        pifi->ifi_name[IFI_NAME-1] = '\0';

        /* Get sock address */
        sinptr = (struct sockaddr_in *)&ifr->ifr_addr;
        pifi->ifi_addr = xcalloc(1, sizeof(struct sockaddr_in));
        memcpy(pifi->ifi_addr, sinptr, sizeof(struct sockaddr_in));

        /* Get mac address */
#ifdef SIOCGARP
        struct arpreq arpreq;
        if (sdlname == NULL || strcmp(sdlname, ifr->ifr_name) != 0)
        {
            idx = 0;

            memcpy((struct sockaddr_in *)&arpreq.arp_pa,
                    pifi->ifi_addr, sizeof(struct sockaddr_in));
            if (ioctl(sockfd, SIOCGARP, &arpreq) >= 0) {
                haddr = arpreq.arp_ha.sa_data;
                hlen = 6;
            } else {
                hlen = 0;
            }
        }
#endif
        pifi->ifi_index = idx;
        pifi->ifi_hlen = hlen;
        if (pifi->ifi_hlen > IFI_HADDR)
            pifi->ifi_hlen = IFI_HADDR;
        if (hlen)
            memcpy(pifi->ifi_haddr, haddr, hlen);
#ifdef SIOCGIFBRDADDR
        if (flags & IFF_BROADCAST) {
            xioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy);
            sinptr = (struct sockaddr_in *)&ifrcopy.ifr_broadaddr;
            pifi->ifi_brdaddr = xcalloc(1, sizeof(struct sockaddr_in));
            memcpy(pifi->ifi_brdaddr, sinptr, sizeof(struct sockaddr_in));
        }
#endif

#ifdef SIOCGIFDSTADDR
        if (flags & IFF_POINTOPOINT) {
            xioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy);
            sinptr = (struct sockaddr_in *)&ifrcopy.ifr_dstaddr;
            pifi->ifi_dstaddr = xcalloc(1, sizeof(struct sockaddr_in));
            memcpy(pifi->ifi_dstaddr, sinptr, sizeof(struct sockaddr_in));
        }
#endif
    }
    free(buf);
    return pifihead;
}


struct ifi_info *Get_ifi_info(void)
{
	struct ifi_info *ptr;
	if ((ptr = get_ifi_info()) == NULL)
		err_sys("get_ifi_info() error");

	return(ptr);
}
