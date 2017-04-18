#include "rdt.h"

void rdt_fin(void)
{
	int n;
	struct conn_user *cptr;
	cptr = conn_user;

        /* Send Fin and wait Ack */
	if ((n = rexmt_pkt(cptr, RDT_FIN, NULL, 0)) < 0)
		err_sys("rexmt_pkt() error");

        /* Notify RDT process to cleanup */
        /* ... */

}


