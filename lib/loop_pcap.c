#include "tcpi.h"

pcap_t *Pt;
sigjmp_buf jumpbuf;

static void sig_int(int signo)
{
#ifdef HAVE_PCAP_BREAKLOOP
	pcap_breakloop(Pt);
#else
	siglongjmp(jumpbuf, 1);
#endif
}

void
loop_pcap(pcap_t *pt, struct bpf_program *bp, handler callback, int cnt)
{
        int rt;
	Pt = pt;

	if (signal_intr(SIGINT, sig_int) == SIG_ERR)
		err_sys("signal SIGINT error");

	if (sigsetjmp(jumpbuf, 1) != 0) {
                rt = -2;
		goto INTR;
        }

        /*
         * In older versions of libpcap,
         * the behavior when cnt was 0 was undefined
         */
        rt = pcap_loop(pt, cnt, callback, NULL);
INTR:
        switch (rt) {
                case 0:
                        fprintf(stderr, "pcap_loop exited\n");
                        break;
                case -1:
                        err_quit("pcap_loop: %s", pcap_geterr(pt));
                case -2:
                        fprintf(stderr, "\npcap_loop was interrupted\n");
                        break;
                default:
                        ;
	}

#ifdef HAVE_PCAP_FREECODE
        pcap_freecode(bp);
#endif
        pcap_close(pt);

        return;
}
