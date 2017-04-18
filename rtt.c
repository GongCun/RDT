#include "rtt.h"

#define RTT_RTOCALC(ptr) \
        ((ptr)->rtt_srtt + (4.0 * (ptr)->rtt_rttvar))

static float rtt_range(float rto)
{
        if (rto < RTT_RXMIN)
                rto = RTT_RXMIN;
        else if (rto > RTT_RXMAX)
                rto = RTT_RXMAX;
        return (rto);
}

void rtt_init(struct rtt_info *ptr)
{
        struct timeval tv;

        if (gettimeofday(&tv, NULL) < 0)
                err_sys("gettimeofday() error");
        ptr->rtt_base = tv.tv_sec;
        ptr->rtt_ts = 0;
        ptr->rtt_rtt = 0;
        ptr->rtt_srtt = 0;
        ptr->rtt_rttvar = 0.75;
        /* First RTO at (srtt + 4*rttvar) = 3 seconds */
        ptr->rtt_rto = rtt_range(RTT_RTOCALC(ptr));
}

uint32_t rtt_setts(struct rtt_info *ptr)
{
        uint32_t ts;
        struct timeval tv;
        if (gettimeofday(&tv, NULL) < 0)
                err_sys("gettimeofday() error");
        ts = ((tv.tv_sec - ptr->rtt_base) * 1000) + (tv.tv_usec / 1000);
        return (ts);
}

void rtt_newpack(struct rtt_info *ptr)
{
        ptr->rtt_nrexmt = 0;
}

int rtt_start(struct rtt_info *ptr)
{
	return ((int)(ptr->rtt_rto + 0.5));	/* round float to int */
	/* for return value can be used as: alarm(rtt_start(&foo)); */
}

/*
 * A response was received. Stop the timer and update the appropriate values
 * in the structure based on the packet's RTT. We calculate the RTT, then
 * update the estimators of the RTT and its mean deviation. This function
 * should be called right after turning off the timer with alarm(0). or right
 * a timeout occurs. Because of the retransmission ambiguity problem, when a
 * reply is received for a request that was retransmitted, _DO_NOT_ use the
 * measured RTT, reuse the RTO for the next packet.
 */
void rtt_stop(struct rtt_info *ptr)
{
        double delta;

        if (ptr->rtt_nrexmt > 0) {
                ptr->rtt_nrexmt = 0;
                ptr->rtt_srtt = 0;
                ptr->rtt_rttvar = 0.75;
                return;
        }

        ptr->rtt_rtt = (rtt_setts(ptr) - ptr->rtt_ts) / 1000.0; /* measured RTT in sec */
        /*
         * Update our estimators of RTT and mean deviation of RTT. */
        delta = ptr->rtt_rtt - ptr->rtt_srtt;
        ptr->rtt_srtt += delta / 8; /* g = 1/8 */
        if (delta < 0)
                delta = -delta; /* |delta| */
        ptr->rtt_rttvar += (delta - ptr->rtt_rttvar) / 4; /* h = 1/4 */
        ptr->rtt_rto = rtt_range(RTT_RTOCALC(ptr));

}

/*
 * A timeout has occured.
 * Return -1 if it's time to give up, else return 0.
 */
int rtt_timeout(struct rtt_info *ptr)
{
        ptr->rtt_rto = rtt_range(ptr->rtt_rto * 2);
        if (++ptr->rtt_nrexmt > RTT_MAXNREXMT)
                return (-1);
        return (0);
}

/*
 * Print debugging information.
 */
void rtt_debug(struct rtt_info *ptr)
{
	fprintf(stderr, "rtt = %.3f, srtt = %.3f, rttvar = %.3f, rto = %.3f, nrexmt = %d\n",
		ptr->rtt_rtt, ptr->rtt_srtt, ptr->rtt_rttvar, ptr->rtt_rto, ptr->rtt_nrexmt);
	fflush(stderr);
}
