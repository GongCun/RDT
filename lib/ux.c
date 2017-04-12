#include "tcpi.h"
#define QLIEN 10

/*
 * Create a server endpoint of a connection.
 * Returns fd if all OK, <0 on error.
 */
int ux_listen(const char *name)
{
        int fd, err, len;
        struct sockaddr_un un;

        if (strlen(name) >= sizeof(un.sun_path)) {
                errno = ENAMETOOLONG;
                return(-1);
        }

        /* Create a UNIX domain stream socket */
        if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
                return(-1);

        unlink(name); /* in case it alraedy exists */

        bzero(&un, sizeof(struct sockaddr_un));
        un.sun_family = AF_UNIX;
        strcpy(un.sun_path, name);
        len = offsetof(struct sockaddr_un, sun_path) + strlen(name);

        if (bind(fd, (struct sockaddr *)&un, len) < 0)
                goto discard;

        if (listen(fd, QLIEN) < 0)
                goto discard;

        return(fd);

discard:
        err = errno;
        close(fd);
        errno = err;
        return(-1);

}


/*
 * Wait for a client connection to arrive, and accept it.
 * Return new fd if all OK, <0 on error.
 */
int ux_accept(int listenfd)
{
        int clifd;
        socklen_t len;
        struct sockaddr_un un;

        if ((clifd = accept(listenfd, (struct sockaddr *)&un, &len)) < 0)
                return(-1);

        return(clifd);
}


/*
 * Create a client endpoint and connect to server.
 * Returns fd if all OK, <0 on error.
 */
int ux_conn(const char *name)
{
        int fd, len, err;
        struct sockaddr_un un;

        if (strlen(name) > sizeof(un.sun_path)) {
                errno = ENAMETOOLONG;
                return(-1);
        }

        if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
                return(-1);

        /* Fill socket address structure with server's address */
        bzero(&un, sizeof(un));
        un.sun_family = AF_UNIX;
        strcpy(un.sun_path, name);
        len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
        if (connect(fd, (struct sockaddr *)&un, len) < 0) {
                err = errno;
                close(fd);
                errno = err;
                return(-1);
        }

        return(fd);
}

/*
 * Unix domain datagram socket are similar to UDP socket.
 */
int ux_serv(const char *name)
{
        int fd, err, len;
        struct sockaddr_un un;

        if (strlen(name) >= sizeof(un.sun_path)) {
                errno = ENAMETOOLONG;
                return(-1);
        }

        /* Create a UNIX domain datagram socket */
        if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
                return(-1);

        unlink(name); /* in case it alraedy exists */

        bzero(&un, sizeof(struct sockaddr_un));
        un.sun_family = AF_UNIX;
        strcpy(un.sun_path, name);
        len = offsetof(struct sockaddr_un, sun_path) + strlen(name);

        if (bind(fd, (struct sockaddr *)&un, len) < 0) {
                err = errno;
                close(fd);
                errno = err;
                return(-1);
        }

        return(fd);
}
int ux_cli(const char *name, struct sockaddr_un *un)
{
        int fd;

        if (strlen(name) > sizeof(un->sun_path)) {
                errno = ENAMETOOLONG;
                return(-1);
        }

        if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
                return(-1);

        /* Fill socket address structure with server's address */
        bzero(un, sizeof(struct sockaddr_un));
        un->sun_family = AF_UNIX;
        strcpy(un->sun_path, name);

        return(fd);
}
