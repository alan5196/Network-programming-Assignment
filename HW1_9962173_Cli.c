#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define FNAMELEN 256
#define PNAMELEN 512
#define PKTLIMIT 2048

int sockfd;
struct sockaddr_in dest;

void conct(char *ip , char * port){
    /* socket descriptor */
    sockfd = socket(PF_INET, SOCK_STREAM,0);
    /* initialize sockaddr */
    bzero(&dest,sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(atoi(port));
    inet_aton(ip, &dest.sin_addr);
    
    /* connect to server */
    if(connect(sockfd, (struct sockaddr*)&dest, sizeof(dest)) == -1)
        perror("Connect");
}
void findDirectory(){
    DIR *dir;
    struct dirent *ptr;
    struct stat buf;
    int count,i;
    char fileName[FNAMELEN];
    // DL
    dir = opendir("./Download");
    printf("/Download/:\n");
    while((ptr = readdir(dir))!=NULL)
        printf("%s\n", ptr -> d_name);
    closedir(dir);
    // UL
    send(sockfd, "i", sizeof(char), 0);
    printf("/Upload/:\n");
    recv(sockfd, &count, sizeof(count), 0);
    for(i = 0; i < count; i++){
        recv(sockfd, fileName, sizeof(fileName), 0);
        printf("%s\n", fileName);
    }
}

void printFunc(){
    printf("1. File Info.\n2. Download File\n3. Upload File\n4. Exit\n");
}
int filesize(char path[]){
    struct stat filestat;
    if (lstat(path, &filestat) < 0){
        exit(1);
    }
    if(filestat.st_size%PKTLIMIT){
        return (filestat.st_size/PKTLIMIT+1);
    }
    else{
        return (filestat.st_size/PKTLIMIT);
    }
}
void upload(){
    char fileName[FNAMELEN],buf[PKTLIMIT];
    FILE *uploadFile;
    char path[PNAMELEN] = "./Download/";
    int n, t, i;
    printf("Enter the filename:");
    scanf("%s",fileName);
    strcat(path,fileName);
    uploadFile=fopen(path,"r");
    if (!uploadFile) {
        perror("fopen");
        exit(1);
    }
    else{
        send(sockfd, "u", sizeof(char), 0);
        t = filesize(path);
        send(sockfd, &t, sizeof(t), 0);
        send(sockfd, fileName, sizeof(fileName), 0);
        for (i=0; i<t; i++) {
            n = fread(buf, sizeof(char), sizeof(buf), uploadFile);
            n = write(sockfd, buf, n);
        }
    }
    fclose(uploadFile);
}
void download(){
    char fileName[FNAMELEN],buf[PKTLIMIT];
    FILE *downloadFile;
    int n, t, i;
    char path[PNAMELEN] = "./Download/";
    printf("Enter the filename:");
    scanf("%s", fileName);
    strcat(path,fileName);
    downloadFile = fopen(path,"w");
    if (!downloadFile) {
        perror("fopen");
        exit(1);
    }else{
        send(sockfd, "d", sizeof(char), 0);
        send(sockfd, fileName, sizeof(fileName), 0);
        recv(sockfd, &t, sizeof(t), 0);
        for (i=0; i<t; i++) {
            n = read(sockfd, buf, sizeof(buf));
            fwrite(buf, sizeof(char), n, downloadFile);
        }
    }
    fclose(downloadFile);
}
int main(int argc, char* argv[]) {
    int ch;
    struct stat bf;
    /* check if folder exist*/
    if(stat("Download/",&bf) != 0)
        system("mkdir Download");
    conct(argv[1], argv[2]);
    /* menu */
    while (printFunc(), scanf("%d",&ch)){
        switch (ch) {
            case 1:
                findDirectory();
                break;
            case 2:
                download();
                printf("Download Complete!\n");
                break;
            case 3:
                upload();
                break;
            case 4:
                send(sockfd, "t", sizeof(char), 0);
                close(sockfd);
                printf("Terminated\n");
                return 0;
            default:
                break;
        }
    }
}
