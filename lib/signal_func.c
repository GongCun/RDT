#include "tcpi.h"

signal_func_t signal_intr(int sig, signal_func_t func)
{
        struct sigaction act, oact;
        act.sa_handler = func;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
        if (sigaction(sig, &act, &oact) < 0)
                return SIG_ERR;
        return(oact.sa_handler);
}

signal_func_t xsignal(int sig, signal_func_t func)
{
        struct sigaction act, oact;
        act.sa_handler = func;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        if (sig == SIGALRM) {
#ifdef SA_INTERRUPT
                act.sa_flags |= SA_INTERRUPT;
#endif
        } else {
#ifdef SA_RESTART
                act.sa_flags |= SA_RESTART;
#endif
        }
        if (sigaction(sig, &act, &oact) < 0)
                return SIG_ERR;
        return(oact.sa_handler);
}
