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

#define FNAMELEN 256
#define PNAMELEN 512
#define PKTLIMIT 2048

int sockfd;
struct sockaddr_in dest;

void sig_chld(int signo) {
    pid_t pid;
    int stat;
    while((pid = waitpid(-1, &stat, WCONTINUED)) > 0)
        return;
}

void conct(char *port){
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    bzero(&dest,sizeof(dest));
    dest.sin_family = PF_INET;
    dest.sin_port = htons(atoi(port));
    dest.sin_addr.s_addr = INADDR_ANY;
    bind(sockfd, (struct sockaddr*)&dest, sizeof(dest));
    listen(sockfd, 100);
}

void sendDir(int clinet_sockfd){
    DIR *dir;
    struct dirent *ptr;
    int n = 0;
    char fileName[FNAMELEN];
    /* read information of folder */
    dir = opendir("./Upload");
    while((ptr = readdir(dir)) != NULL) n++;
    send(clinet_sockfd, &n, sizeof(n), 0);
    dir = opendir("./Upload");
    while((ptr = readdir(dir))!=NULL){
        strcpy(fileName, ptr->d_name);
        send(clinet_sockfd, fileName, sizeof(fileName), 0);
    }
    closedir(dir);
}
int filesize(char path[]){
    struct stat filestat;
    if ( lstat(path, &filestat) < 0)
        exit(1);
    if(filestat.st_size%PKTLIMIT)
        return (filestat.st_size/PKTLIMIT+1);
    else
        return (filestat.st_size/PKTLIMIT);
}

void sendFile(int *clinet_sockfd){
    char fileName[FNAMELEN],buf[PKTLIMIT];
    FILE *sendF;
    int t,i,n;
    char path[PNAMELEN]="./Upload/";
    recv(*clinet_sockfd,fileName,sizeof(fileName),0);
    strcat(path,fileName);
    t = filesize(path);
    send(*clinet_sockfd, &t, sizeof(t), 0);
    sendF=fopen(path,"r");
    if (!sendF) {
        perror("fopen");
        exit(1);
    }else{
        for (i=0; i<t; i++) {
            n = fread(buf, sizeof(char), sizeof(buf), sendF);
            write(*clinet_sockfd, buf, n);
        }
    }
    fclose(sendF);
    return;
}
void receiveFile(int *clinet_sockfd){
    char fileName[FNAMELEN],buf[PKTLIMIT];
    FILE *receiveF;
    int n,t,i;
    char path[PNAMELEN]="./Upload/";
    recv(*clinet_sockfd, &t, sizeof(t), 0);
    recv(*clinet_sockfd,fileName,sizeof(fileName),0);
    strcat(path,fileName);
    receiveF = fopen(path,"w");
    if (!receiveF) {
        perror("fopen");
        exit(1);
    }else{
        for (i=0; i<t; i++) {
            n = read(*clinet_sockfd, buf, sizeof(buf));
            fwrite(buf, sizeof(char), n, receiveF);
        }
    }
    fclose(receiveF);
    return;
}
int main(int argc, char* argv[]){
    int clinet_sockfd,addrlen;
    struct sockaddr_in client_addr;
    char type;
    struct stat bf;
    pid_t childpid;
    if(stat("Upload/", &bf)!=0)  // Chk folder existence
        system("mkdir Upload");
    conct(argv[1]);
    bzero(&client_addr,sizeof(client_addr));
    addrlen = sizeof(client_addr);
    signal(SIGCHLD, sig_chld);
    while(1) {
        clinet_sockfd = accept(sockfd,(struct sockaddr *)&client_addr, &addrlen);
        /* print client address and port number */
        printf("Client: %s:%d has connected\n", inet_ntoa(client_addr.sin_addr), htons(dest.sin_port));
        if ( (childpid = fork()) == 0) {
            while (1) {
                recv(clinet_sockfd, &type,sizeof(type), 0);
                switch (type) {
                    case 'i':
                        sendDir(clinet_sockfd);
                        break;
                    case 'u':
                        receiveFile(&clinet_sockfd);
                        printf("File Received\n");
                        break;
                    case 'd':
                        sendFile(&clinet_sockfd);
                        break;
                    case 't':
                        printf("%s Terminate!\n",inet_ntoa(client_addr.sin_addr));
                        close(clinet_sockfd);
                        return 0;
                        break;
                }
            }
        }
    }
}
