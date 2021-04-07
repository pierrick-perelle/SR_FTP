/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include "utils.h"

void request_file(int connfd,rio_t rio,char* fileName){
    int fd;
    size_t n;
    char buf[MAXBUF];
    rio_t riof;

    printf("nom du fichier recherché : %s \n",fileName);

    fd=open(fileName,O_RDONLY,0);
        
    //Si impossible d'ouvrir fichier(n'existe pas), envoie erreur
    if(fd<0){
        Rio_writen(connfd, "NOT_FOUND\n", 10);
        printf("Erreur: %s n'existe pas\n", fileName);
        return;
    }
    //sinon envoie ok
    Rio_writen(connfd, "ok", 3);

    //envoie au client la taille du fichier à télécharger
    int tailleTotale = file_size(fileName);
    printf("Téléchargement: %s (%d octets).\n", fileName,tailleTotale);
    Rio_writen(connfd, &tailleTotale, sizeof(int));

    //envoie au client de la date de dernière modification du fichier
    time_t dateModification;
    dateModification = file_date_fd(fd);
    Rio_writen(connfd, &dateModification, sizeof(time_t));
    
    //reception du numéro de l'octet à partir duquel envoyer le fichier au client (le client ne retéléchargera pas la partie du fichier qu'il possède déjà)
    int departEnvoie = 0;
    Rio_readnb(&rio, &departEnvoie, sizeof(int));
    
    Rio_readinitb(&riof, fd);
    int i;
    char tmp;
    //on se place au bon endroit dans le fichier 
    for(i=0;i<departEnvoie;i++){
        Rio_readnb(&riof,&tmp,1);
    }

    //Envoie du fichier
    printf("Envoie de %d octets...\n",tailleTotale-departEnvoie);
    while((n=Rio_readnb(&riof, buf, MAXBUF)) != 0){
        if(rio_writen(connfd, buf, n)<0){
        break;
        }
    }
    printf("Envoie Terminé.\n");
    Close(fd);
    return;
}

void ftp(int connfd){
    size_t n;
    char buf[MAXLINE];
    char* ligne;
    rio_t rio;
    char SERV_DIR[100] =  {'\0'};
    strcat(SERV_DIR,"./Serveur/");

    Rio_readinitb(&rio, connfd);

    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {

        printf("received : %s \n",buf);
        ligne = strtok(buf," \n"); //buf[0]
        if(strcmp(ligne,"get")==0){
            printf("get detected\n");
            ligne=strtok(NULL," \n"); //buf[1]
            if(buf!=NULL){
                printf("starting request process... \n");
                request_file(connfd,rio,strcat(SERV_DIR,ligne));
            }
            else{
                fprintf(stderr,"Utilisation: get <nom_de_fichier>\n");
            }
        }
        else{
            fprintf(stderr,"commande introuvable %s\n",ligne);
            Rio_writen(connfd,"tg ça marche pas",17);
        }
        fprintf(stderr,"server received %u bytes\n", (unsigned int)n);
        Rio_writen(connfd, ligne, n);
    }
}

