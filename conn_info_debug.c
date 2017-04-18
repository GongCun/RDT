#include "rdt.h"

void conn_info_debug(struct conn_info *ciptr)
{
        fprintf(stderr, "pid = %ld, cact = %d, ", (long)ciptr->pid, ciptr->cact);
        fprintf(stderr, "src = %s, ", inet_ntoa(ciptr->src));
        fprintf(stderr, "dst = %s, ", inet_ntoa(ciptr->dst));
        fprintf(stderr, "scid = %d, dcid = %d\n", ciptr->scid, ciptr->dcid);
        fflush(stderr);
        return;
}
