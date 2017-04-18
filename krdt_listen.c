#include "rdt.h"
#include "rtt.h"

/* listen() for a connection. If RDT_REQ has already arrived,
 * and reply a RDT_ACK, a passive connection will be established.
 * The variables will be initialized:
 * src          source address
 * scid         source conn id
 * cstate       CLOSED->LISTEN
 */

int krdt_listen(struct in_addr src, int scid, pid_t pid)
{
        int i;
        struct conn *cptr;

        for (i = 0; i < MAX_CONN; i++)
                if (conn[i].cstate != CLOSED && conn[i].scid == scid) {
			fprintf(stderr, "krdt_listen() i = %d\n", i);
			conn_debug(&conn[i]);
                        err_quit("Can't start listen() because the SCID is in use.");
		}

        for (i = 0; i < MAX_CONN; i++)
                if (conn[i].cstate == CLOSED)
                        break;
        if (i >= MAX_CONN)
                err_quit("Can't start listen() because the connection is full.");

        cptr = &conn[i];
        memcpy(&cptr->src, &src, sizeof(src));
        cptr->scid = scid;
        cptr->sndfd = open_fifo(pid, "snd");
        cptr->rcvfd = open_fifo(pid, "rcv");
        cptr->cstate = LISTEN;

        return (i);
}
