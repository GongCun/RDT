#include "rdt.h"

void conn_user_debug(struct conn_user *cuptr)
{
        fprintf(stderr, "sfd = %d, sndfd = %d, rcvfd = %d, ",
                        cuptr->sfd, cuptr->sndfd, cuptr->rcvfd);
        fprintf(stderr, "mss = %d, ", cuptr->mss);
        fprintf(stderr, "src = %s, ", inet_ntoa(cuptr->src));
        fprintf(stderr, "dst = %s, ", inet_ntoa(cuptr->dst));
        fprintf(stderr, "scid = %d, dcid = %d\n", cuptr->scid, cuptr->dcid);
        fflush(stderr);
        return;
}
