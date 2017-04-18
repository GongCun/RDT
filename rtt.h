#ifndef _RTT_H
#define _RTT_H

#include "tcpi.h"

struct rtt_info {
	float rtt_rtt;		/* most recent measured RTT, in sec */
	float rtt_srtt;		/* smoothed RTT estimator, in sec */
	float rtt_rttvar;	/* smoothed mean deviation, in sec */
	float rtt_rto;		/* current RTO to use, in sec */
	int rtt_nrexmt;		/* # times retransmitted: 0, 1, 2, ... */
	uint32_t rtt_base;	/* sec since 1/1/1970 at start */
	uint32_t rtt_ts;	/* millisec from the base */
} rtt_info;


#define RTT_RXMIN 2		/* min retransmit timeout value, in sec */
#define RTT_RXMAX 20		/* max retransmit timeout value, in sec */
#define RTT_MAXNREXMT 9		/* max # times to retransmit */

void rtt_init(struct rtt_info *);
uint32_t rtt_setts(struct rtt_info *);
void rtt_newpack(struct rtt_info *ptr);
int rtt_start(struct rtt_info *ptr);
void rtt_stop(struct rtt_info *ptr);
int rtt_timeout(struct rtt_info *ptr);
void rtt_debug(struct rtt_info *ptr);

#endif
