/*
 * Copyright (c) 2017 Cun Gong
 * Copyright (c) 2003 W. Richard Stevens, Bill Fenner, Andrew M. Rudoff
 */
#ifndef __TCPI_H
#define __TCPI_H

#include "../config.h"

#define MAXLINE 4096 /* max text line length */
#define BUFFSIZE 8192 /* buffer size for reads and writes */
#define MAXLEN 65535 /* max IP packet length */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h> /* uint8_t, uint16_t, ... */
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <setjmp.h>
#include <signal.h>
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <sys/socket.h>
#include <netdb.h> /* gethostbyname(), ... */
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#ifdef _LINUX
#define __FAVOR_BSD 1
#endif
#include <netinet/tcp.h>
#include <netinet/udp.h> /* struct udphdr */
#include <net/if.h>
#ifdef HAVE_NET_ARP_H
#include <net/if_arp.h> /* struct arpreq */
#endif
#ifdef HAVE_NET_IF_DL_H
#include <net/if_dl.h> /* struct sockaddr_dl */
#endif
#include <net/route.h> /* struct rt_msghdr ... */
#include <arpa/inet.h>
#ifdef HAVE_PCAP_H
#include <pcap.h>
#endif
#if defined(_AIX) || defined(_AIX64)
#include <net/bpf.h>
#include <netinet/if_ether.h>
#else
#include <net/ethernet.h>
#endif
#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif
#include <sys/un.h>
#include <sys/stat.h>

#define CKSUM_CARRY(x) \
        (x = (x >> 16) + (x & 0xffff), (~(x + (x >> 16)) & 0xffff))

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

#ifndef IP_OFFMASK
#define IP_OFFMASK 0x1fff
#endif

#ifndef CMSG_LEN
#define CMSG_LEN(size) (sizeof(struct cmsghdr) + (size))
#endif
#ifndef CMSG_SPACE
#define CMSG_SPACE(size) (sizeof(struct cmsghdr) + (size))
#endif


/* Error handler functions */
void err_dump(const char *, ...);
void err_msg(const char *, ...);
void err_quit(const char *, ...);
void err_ret(const char *, ...);
void err_sys(const char *, ...);

/* Common unix function */
int xioctl(int, unsigned long, void *);
int xsocket(int, int, int);
void *xmalloc(size_t);
void *xcalloc(size_t, size_t);

/*
 * Get_ifi_info
 */
#define IFI_NAME 16 /* same as IFINAMSIZ in <net/if.h> */
#define IFI_HADDR 8

struct ifi_info {
    char ifi_name[IFI_NAME]; /* interface name, null-terminated */
    short ifi_index; /* interface name */
    short ifi_mtu; /* interface MTU */
    u_char ifi_haddr[IFI_HADDR]; /* hardware address */
    u_short ifi_hlen; /* bytes# in hardware address: 0, 6, 8 */
    short ifi_flags; /* IFF_xxx constants from <net/if.h> */
    struct sockaddr *ifi_addr; /* primary address */
    struct sockaddr *ifi_brdaddr; /* broadcast address */
    struct sockaddr *ifi_dstaddr; /* P2P destination address */
    struct ifi_info *ifi_next; /* next of these structures */
};
struct ifi_info *get_ifi_info(void); /* only for inet4 */
struct ifi_info *Get_ifi_info(void); /* wrap get_ifi_info() */
void free_ifi_info(struct ifi_info *);

/* Check sum */
uint16_t checksum(uint16_t *, int);
int do_checksum(u_char *buf, int protocol, int len);
uint32_t in_checksum(uint16_t * addr, int len);


/* pcap functions */
typedef void (*handler)(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes);
pcap_t *open_pcap(const char *device, int to_ms, char *cmd, int *linktype, struct bpf_program *bp);
void dispatch_pcap(pcap_t *pt, struct bpf_program *, handler callback, int cnt);
void loop_pcap(pcap_t *pt, struct bpf_program *bp, handler callback, int cnt);


/* signal functions */
typedef void (*signal_func_t)(int);
signal_func_t signal_intr(int sig, signal_func_t func);
signal_func_t Signal(int sig, signal_func_t func);

/*
 * For Unique Connections of Unix Domain Sockets.
 */
#ifndef offsetof
#define offsetof(type, f) ((size_t) \
                ((char *)&((type *)0)->f - (char *)(type *)0))
#endif
int ux_listen(const char *);
int ux_accept(int);
int ux_conn(const char *);
int ux_serv(const char *);
int ux_cli(const char *, struct sockaddr_un *);


#endif /* __TCPI_H */


