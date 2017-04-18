#include "rdt.h"
#include "rtt.h"

/* connect() for connect to remote process by sending RDT_REQ packet,
 * If received RDT_ACC packet, a active connection would be established.
 * The variables will be initialized:
 * src          sour address
 * dst          dest address
 * scid         sour conn id - search for conn[] to find unused scid
 * dcid         dest conn id
 * cstate       CLOSED->WAITING
 * fifo         fifo FD
 */

int krdt_connect(struct in_addr dst, int scid, int dcid, pid_t pid)
{
        int i, j;
        struct conn *cptr;
        struct in_addr src;
	struct conn_info conn_info;
        
        /* Choose a scid internal if 
         * not in the legal range.
         */
	if (scid < 0 || scid >= MAX_CONN) {
		for (i = 0; i < MAX_CONN; i++) {
			for (j = 0; j < MAX_CONN; j++) {
				if (i == conn[j].scid && conn[j].cstate != CLOSED)
					break;
			}
			if (j >= MAX_CONN)	/* not in use */
				break;
		}
		if (i >= MAX_CONN)
			err_quit("Can't start connect() because the conn-id is full.");
		scid = i;
	} else {		/* Check the scid if in use */
		for (i = 0; i < MAX_CONN; i++)
			if (conn[i].cstate != CLOSED && conn[i].scid == scid)
				err_quit("Can't start connect() because the SCID is in use.");
	}

        for (i = 0; i < MAX_CONN; i++)
                if (conn[i].cstate == CLOSED)
                        break;
        if (i >= MAX_CONN)
                err_quit("Can't start connect() because the connection is full.");

        cptr = &conn[i];
        cptr->scid = scid;
        cptr->dcid = dcid;
        src = get_addr(dst);
        memcpy(&cptr->src, &src, sizeof(src));
        memcpy(&cptr->dst, &dst, sizeof(dst));

        cptr->sndfd = open_fifo(pid, "snd");
        cptr->rcvfd = open_fifo(pid, "rcv");
	bzero(&conn_info, sizeof(conn_info));
	conn_info.scid = scid;
	pass_pkt(cptr->sndfd, &conn_info, NULL, 0);
        cptr->cstate = WAITING;

        return (i);
}
