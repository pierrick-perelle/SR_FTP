/*
 * echoserveri.c - An iterative echo server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define NBPROC 5

void ftp(int connfd);
void server_get(int connfd,rio_t rio,char* fileName);


void no_chld(){
  pid_t pid;
  while((pid = waitpid(-1, NULL, WNOHANG)) > 0){}
  return;
}

/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv){

    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    signal(SIGCHLD, no_chld);
    
    if (argc != 1) {
        fprintf(stderr, "usage error\n");
        exit(0);
    }
    //port = atoi(argv[1]);
    port = 2198;
    
    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port);
    int i;
    for(i=0;i<NBPROC;i++){
        if(fork()==0){
            while (1) {
                
                connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

                /* determine the name of the client */
                Getnameinfo((SA *) &clientaddr, clientlen,client_hostname, MAX_NAME_LEN, 0, 0, 0);
                
                /* determine the textual representation of the client's IP address */
                Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,INET_ADDRSTRLEN);
                
                printf("server connected to %s (%s)\n", client_hostname,client_ip_string);

                //traiter ici
                ftp(connfd);

                Close(connfd);
            }
        }
    }
    while(1){};
    return 0;
}

