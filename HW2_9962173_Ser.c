
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
#define LISTENQ 100
ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t readline(int fd, void *vptr, size_t maxlen);
int logOut(int client);
void chatting(char *userName,char *content,int sockfd);
void signInList(int sockfd);
void Broadcast(char *content);
void accountInitial();
int CommandProcess(char *line,int sockfd,int client);
struct userInfo{
    char userName[20];
    char password[20];
    char content[1024];
    int isUse;
    int online;
    int client;
    int mesg;
}dataBase[100];
 int client[FD_SETSIZE];
int main(int argc, char* argv[]){
    int i, maxi, maxfd, listenfd, connfd, sockfd,x;
    int nready;
    int dropPro;
    ssize_t n;
    fd_set rset, allset;
    char line[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(5566);
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    maxfd = listenfd; /* initialize */
    maxi = -1; /* index into client[] array */
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1; /* -1 indicates available entry */
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    accountInitial();
    for ( ; ; ) {
        rset = allset; /* structure assignment */
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(listenfd, &rset)) {
            /* new client connection */
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
            for (i = 0; i < FD_SETSIZE; i++)
                if (client[i] < 0) {
                client[i] = connfd; /* save descriptor */
                break;
            }
            if (i == FD_SETSIZE)
                printf("Too many clients");
            FD_SET(connfd, &allset); /* add new descriptor to set */
            if (connfd > maxfd)
                maxfd = connfd; /* for select */
            if (i > maxi)
                maxi = i; /* max index in client[] array */
            if (--nready <= 0)
                continue; /* no more readable descriptors*/
        }
        
        for(i=0;i<=maxi;i++){ /*checkallclientsfordata*/
            if ( (sockfd = client[i]) < 0)
                continue;
            
            if (FD_ISSET(sockfd, &rset)) {
                if ( (n = read(sockfd, line, MAXLINE)) == 0) {
                        /* connection closed by client */
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                }
                else{
                    //writen(sockfd, line, n);
                   // dropPro=(rand() % 10) +1;
                   // if(dropPro<=3){
                        if(x=CommandProcess(line,sockfd,i)){
                            close(sockfd);
                            FD_CLR(sockfd, &allset);
                            client[i] = -1;
                        }
                   // }
                }
                if (--nready <= 0)
                    break;/* no more readable descriptors */
                }
            }
        }
    }
void accountInitial(){
    int i;
    for(i=0;i<100;i++){
        dataBase[i].isUse=0;
        dataBase[i].online=0;
        dataBase[i].mesg=0;
    }
}
int CommandProcess(char *line,int sockfd,int client){
    char O[1];
    char A[21];
    char B[1002];
    static onlineNum = 1;   //record number of online user
    static accountNum = 0;  //record number of account
    int i;
    char buf[100];
    printf("%c\n",line[0]);
    switch (line[0]) {
        case 'N':
            sscanf(line,"%s %s %s",O,A,B);
            for(i=0;i<100;i++){
                if(!dataBase[i].isUse){
                    dataBase[i].isUse=1;
                    strcpy(dataBase[i].userName,A);
                    strcpy(dataBase[i].password,B);
                    strcpy(buf,"(Account created)");
                    writen(sockfd, buf, strlen(buf));
                    break;
                }
            }
            accountNum++;
            break;
        case 'D':
            sscanf(line,"%s %s %s",O,A,B);
            for(i=0;i<LISTENQ;i++){
                if(strcmp(dataBase[i].userName,A)==0){
                    if(strcmp(dataBase[i].password,B)==0){
                        dataBase[i].isUse=0;
                        dataBase[i].online=0;
                        dataBase[i].mesg=0;
                        strcpy(dataBase[i].content,"");
                        strcpy(dataBase[i].userName,"");
                        strcpy(dataBase[i].password,"");
                        accountNum--;
                        strcpy(buf,"(Account deleted)");
                        writen(sockfd, buf, strlen(buf));
                        
                    }
                    else{
                        strcpy(buf,"(Wrong Password)");
                        writen(sockfd, buf, strlen(buf));
                    }
                    break;
                }
                if(i==LISTENQ-1){
                    strcpy(buf,"(Username not exist)");
                    writen(sockfd, buf, strlen(buf));
                }
            }
            
            break;
        case 'S':
            sscanf(line,"%s %s %s",O,A,B);
            for(i=0;i<LISTENQ;i++){
                if(strcmp(dataBase[i].userName,A)==0){
                    if(strcmp(dataBase[i].password,B)==0){
                        ++onlineNum;
                        dataBase[i].online=1;
                        dataBase[i].client=client;
                        strcpy(buf,"(Sign in successfully)");
                        writen(sockfd, buf, strlen(buf));
                        signInList(sockfd);
                        if(dataBase[i].mesg)
                             writen(sockfd, dataBase[i].content, strlen(dataBase[i].content));
                    }
                    else{
                        strcpy(buf,"(Wrong Password)");
                        writen(sockfd, buf, strlen(buf));
                    }
                    break;
                }
                if(i==LISTENQ-1){
                    strcpy(buf,"(Username doesn't exist)");
                    writen(sockfd, buf, strlen(buf));
                }
            }
            break;
        case 'O':
            sscanf(line,"%s",O);
            if(logOut(client)){
                onlineNum--;
                strcpy(buf,"(Sign out)");
                writen(sockfd, buf, strlen(buf));
                return 1;
            }
            else{
                strcpy(buf,"(You've not signed in)");
                writen(sockfd, buf, strlen(buf));
            }
            break;
        case 'C':
            sscanf(line,"%s %s %s",O,A,B);
            chatting(A,B,sockfd);
            break;
        case 'B':
            sscanf(line,"%s %s",O,B);
            Broadcast(B);
            break;
        default:
            strcpy(buf,"(Try again)");
            writen(sockfd, buf, strlen(buf));
            break;
    }    
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
void signInList(int sockfd){
    int i,sum=-1;
    char a[20];
    for(i=0;i<LISTENQ;i++){
        if(dataBase[i].online){
            if(++sum==0){
                strcpy(a,"\n(Online user):");
                writen(sockfd,a, strlen(a));
            }
            strcpy(a,dataBase[i].userName);
            strcat(a," ");
             writen(sockfd,a, strlen(a));
        }
    }
    
}
int logOut(int client){
    int i;
    for(i=0;i<LISTENQ;i++){
        if(dataBase[i].client==client && dataBase[i].online==1){
            dataBase[i].online=0;
            return 1;
        }
    }
    return 0;
}
void chatting(char *userName,char *content,int sockfd){
    char buf[1024];
    int i;
    for(i=0;i<LISTENQ;i++){
        if(strcmp(dataBase[i].userName,userName)==0){
            if(dataBase[i].online==0){//user is offline
                strcpy(buf,"(Message):");
                strcat(buf,content);
                strcat(buf," ");
                dataBase[i].mesg=1;
                strcat(dataBase[i].content,buf);
                strcpy(buf,dataBase[i].userName);
                strcat(buf," is offline, so the message is saved");
                writen(sockfd,buf,strlen(buf));

            }
            if(dataBase[i].online==1){//user is online
                strcpy(buf,"(Message):");
                strcat(buf,content);
                writen(client[dataBase[i].client],buf,strlen(buf));
                strcpy(buf,dataBase[i].userName);
                strcat(buf," have received");
                writen(sockfd,buf,strlen(buf));
            }
            break;
        }
        if(i==LISTENQ-1){
            strcpy(buf,"(Username doesn't exist)");
            writen(sockfd, buf, strlen(buf));
        }
    }
    
}
void Broadcast(char *content){
    int i;
    char buf[1024];
    for(i=0;i<LISTENQ;i++){
        if(dataBase[i].online==1){
            strcpy(buf,"(Message):");
            strcat(buf,content);
            writen(client[dataBase[i].client],buf,strlen(buf));
        }
    }
}

