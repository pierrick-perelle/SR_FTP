/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"

void ftp(int connfd){
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {

        char** cmd = splitCmd(buf);

        if((strcmp(cmd[0],"get") == 0)){
            FILE* fd;
            char data[MAXBUF];
            if((fd = fopen(cmd[1],"r"))){
                  sprintf(data,"%ld");
                  send(connfd,data,strlen(data),0);
            }else{
                  printf("file not found");
            }
        }
        else{
            fprintf(stderr,"cmd non reconnu");
        }


        printf("server received %u bytes\n", (unsigned int)n);
        Rio_writen(connfd, buf, n);
    }
}
