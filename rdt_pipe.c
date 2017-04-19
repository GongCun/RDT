#include "rdt.h"
#include <sys/mman.h>


void waitchild(void)
{
        pid_t pid;
        while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
                fprintf(stderr, "process %ld terminated\n", (long)pid);
}

static void sigchld(int signo)
{
        waitchild();
}


void rdt_pipe(int fd[2])
{
	int pfd[2];
	pid_t pid;

        if (atexit(waitchild) < 0)
                err_sys("atexit() error");
        if (signal(SIGCHLD, sigchld) == SIG_ERR)
                err_sys("signal() SIGCHLD error");

	if (pipe(pfd) < 0)
		err_sys("pipe() error");
	if ((pid = fork()) < 0)
		err_sys("fork() error");
	if (pid == 0) { /* child process for recv data from network */
		/* recv data from network */
                fprintf(stderr, "fork recv process: %ld\n", (long)getpid());
		close(pfd[0]); 
                rdt_recv(pfd[1]);
		exit(0);
	}
	fd[0] = pfd[0]; /* read only for recv data from network */
	close(pfd[1]); 


	/* Parent continue fork process for send data */

	if (pipe(pfd) < 0)
		err_sys("pipe() error");
	if ((pid = fork()) < 0)
		err_sys("fork() error");
	if (pid == 0) { /* child process for send data to network */
		/* send data to network */
                fprintf(stderr, "fork send process: %ld\n", (long)getpid());
		close(pfd[1]); 
                if (rdt_send(pfd[0]) < 0)
                        err_sys("rdt_send() error");
                exit(0);
	}
	fd[1] = pfd[1]; /* write only for send data to network */
	close(pfd[0]);

	return;
}

