#include "rdt.h"

void waitchild(void)
{
        while (waitpid(-1, NULL, WNOHANG) > 0)
                ;
}


void rdt_pipe(int fd[2])
{
	int n, len;
	int pfd[2];
	pid_t pid;
	char buf[MAXLINE];

        if (atexit(waitchild) < 0)
                err_sys("atexit() error");

	if (pipe(pfd) < 0)
		err_sys("pipe() error");
	if ((pid = fork()) < 0)
		err_sys("fork() error");
	if (pid == 0) { /* child process for recv data from network */
		/* recv data from network */
		close(pfd[0]); 
		while ((len = rdt_recv(buf, MAXLINE)) > 0) {
			if ((n = write(pfd[1], buf, len)) != len &&
                                        errno != EWOULDBLOCK && errno != EAGAIN)
                        {
				err_sys("write() %d bytes, expect %d bytes", n, len);
                        }
                }
                /* rdt_close(); */
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
		close(pfd[1]); 
	        n = conn_user->mss - IP_LEN - RDT_LEN;
		while ((len = read(pfd[0], buf, n)) > 0)
			if ((n = rdt_send(buf, len)) != len)
				err_sys("rdt_send() %d bytes, expect %d bytes", n, len);
                rdt_fin();
                exit(0);
	}
	fd[1] = pfd[1]; /* write only for send data to network */
	close(pfd[0]);

	return;
}

