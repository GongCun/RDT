#ifndef _RDT_H
#define _RDT_H

#include "tcpi.h"
#include <sys/uio.h>

#define MAX_CONN 4
#define IP_LEN 20
#define RDT_LEN sizeof(struct rdthdr)
#define GUESS_MTU 1500 
#undef  IPPROTO_RDT
#define IPPROTO_RDT 143 /* 143-252 (0x8F-0xFC) UNASSIGNED */

#define ranport() ((getpid() & 0xfff) | 0x8000)
#define FILTER "ip[9:1] == 143"
#define PCAP_TIMEOUT 500 /* 500 milliseconds */

#define RDT_UX_SOCK "/tmp/RDTUXSock" /* Unix Domain Socket for User IPC */
#define RDT_FIFO "/tmp/RDTFifo" /* FIFO pipe for User IPC */
#define FILE_MODE 0666
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
#define CLOSE_WAIT 20

#undef MAXLINE
#define MAXLINE 1472 /* 1500 - 20 - 8 */

typedef enum {CLOSED, LISTEN, WAITING, ESTABLISHED} cstate;

/*
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   SRC CID#    |    DST CID#   |   Seq(0/1)    |     Flags     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |         Packet length         |            Checksum           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                             Data...                           |
 *
 */

struct rdthdr {
        uint8_t rdt_scid;
        uint8_t rdt_dcid;
        uint8_t rdt_seq;
#define RDT_SEQ0 0x00
#define RDT_SEQ1 0x01
        uint8_t rdt_flags;
#define RDT_FIN  0x01 /* finish request */
#define RDT_CONF 0x02 /* confirm close, no use */
#define RDT_REQ  0x04 /* connection request */
#define RDT_ACC  0x08 /* connection accept */
#define RDT_DATA 0x10
#define RDT_ACK  0x20 /* data and fin confirm */
        uint16_t rdt_len;
        uint16_t rdt_sum;
};


/* For user process transfer data */
struct conn_user {
        struct in_addr src, dst;
        int scid, dcid;
        int sfd, sndfd, rcvfd;
        unsigned char *sndpkt;
	unsigned char *rcvpkt;
        int mss;
	int seq, ack;
} *conn_user;

/* For exchange info between user and RDT process */
typedef enum {ACTIVE, PASSIVE} cact;
struct conn_info {
        pid_t pid; /* user pid for FIFO's id */
        cact cact; /* passive or active */
        struct in_addr src, dst;
        int scid, dcid;
};

/* For RDT process keep the conn info and FSM state */
struct conn {
        int sndfd, rcvfd;
        cstate cstate;
        struct in_addr src, dst;
        int scid, dcid;
} conn[MAX_CONN];
struct conn *closePtr;

/*--------------------------------*/

struct conn_ret {
        int ret;
        int err;
};

extern char dev[IFNAMSIZ];
extern int linktype;
extern int mtu;

struct in_addr get_addr(struct in_addr dst);
ssize_t make_pkt(struct in_addr src, struct in_addr dst, int scid, int dcid, int seq, int flags, void *data, size_t nbyte, void *buf);
ssize_t to_net(int fd, const void *buf, size_t nbyte, struct in_addr dst);
void from_net(void);
int get_dev(struct in_addr addr, char *dev);
int get_mtu(char *dev);
int chk_chksum(uint16_t *ptr, int len);
int krdt_listen(struct in_addr src, int scid, pid_t pid);
int krdt_connect(struct in_addr dst, int scid, int dcid, pid_t pid);
int pkt_arrive(struct conn *cptr, const u_char *pkt, int len);
int make_sock(void);
int make_fifo(pid_t, const char *);
int open_fifo(pid_t, const char *);
ssize_t rexmt_pkt(struct conn_user *, uint8_t, void *, size_t);
void pkt_debug(const struct rdthdr *);
int rdt_connect(struct in_addr dst, int scid, int dcid);
int rdt_listen(struct in_addr src, int scid);
void conn_debug(struct conn *);
void conn_info_debug(struct conn_info *);
void conn_user_debug(struct conn_user *);

ssize_t get_pkt(int fd, struct conn_info *ciptr, u_char *buf, ssize_t buflen);
ssize_t pass_pkt(int fd, struct conn_info *ciptr, u_char *buf, ssize_t buflen);
ssize_t rdt_send(void *buf, size_t nbyte);
ssize_t rdt_recv(void *buf, size_t nbyte);
void rdt_fin(void); /* close active */
void rdt_close(void); /* close passive */
void rdt_xmit(int fd[2]);
void conn_alloc(void);
void rdt_pipe(int fd[2]);


#endif

