

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#define MAXLINE 1024

ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t readline(int fd, void *vptr, size_t maxlen);
void str_cli(FILE *fp, int sockfd);

int main(int argc, char **argv){
    int sockfd;
    struct sockaddr_in servaddr;
    if (argc != 2)
        printf("Usage: tcpcli <IPaddress>");
    
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(5566);
        inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
        connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
        str_cli(stdin,sockfd);
    
    return 0;
}

ssize_t writen(int fd, const void *vptr, size_t n) {
    ssize_t nleft;
    ssize_t nwritten;
    const char *ptr;
    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0; /* and call write() again */
            else
                return (-1); /* error */
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);
}
ssize_t readline(int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;
    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
    again:
        if ( (rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break; /* newline is stored, like fgets() */
        }
        else if (rc == 0) {
            *ptr = 0;
            return (n - 1); /* EOF, n - 1 bytes were read */
        }
        else {
            if (errno == EINTR)
                goto again;
            return (-1); /* error, errno set by read() */
        }
    }
    *ptr = 0; /* null terminate like fgets() */
    return (n);
}
void str_cli(FILE *fp, int sockfd) {
    int maxfdp1,n,i,c=0;
    fd_set rset;
    char sendline[MAXLINE], recvline[MAXLINE];
    struct timeval timeout={3,0};
    FD_ZERO(&rset); /*initial select*/
    for ( ; ; ) {
        bzero(&recvline,MAXLINE);
        /*fp: I/O file pointer*/
        /*fileno convers fp into descriptor*/
        FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 =(fileno(fp)>sockfd)?fileno(fp)+1:sockfd+1;
        n=select(maxfdp1, &rset, NULL, NULL, &timeout);
        if(n==0 && c>0){// if timeout resend
            printf("Timeout re-send\n");
            goto a;
        }
        if (FD_ISSET(sockfd, &rset)) { /* socket is readable */
            if (read(sockfd, recvline, MAXLINE) == 0){
                close(sockfd);
                FD_CLR(sockfd, &rset);
                return ;
            }
            fputs(recvline, stdout);
            fputs("\n",stdout);
            c--;
        }
     if (FD_ISSET(fileno(fp), &rset)) { /* input is readable */
            if (fgets(sendline, MAXLINE, fp) == NULL)
            return; /* all done */
            c++;
           a:write(sockfd, sendline, strlen(sendline));
        }
    }
}
        
        
        
        
        
