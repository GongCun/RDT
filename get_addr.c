#include "rdt.h"

struct in_addr get_addr(struct in_addr dst)
{
        int sockfd;
        struct sockaddr_in servaddr;
        struct sockaddr_in cliaddr;
        socklen_t len;

        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
                err_sys("socket() error");
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(ranport());
        memcpy(&servaddr.sin_addr, &dst, sizeof(dst));

        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                err_sys("connect() error");

        len = sizeof(cliaddr);
        if (getsockname(sockfd, (struct sockaddr *)&cliaddr, &len) < 0)
                err_sys("getsockname() error");
        return(cliaddr.sin_addr);
}
