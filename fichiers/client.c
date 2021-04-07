/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include "utils.h"
#define PORT 2121

void client_get(int clientfd,rio_t rio,char* file){
    char buf[MAXLINE];
    char filePath[100] = {'\0'};
    char date[20];
    int fd;
    time_t t1, t2;

    char CLI_DIR[100] = {'\0'};
    strcat(CLI_DIR,"./Client/");

    strcpy(filePath,(strcat(CLI_DIR,file)));
    printf("file : %s\n",file);
    printf("file path : %s \n",filePath);
        
    printf("Attente confirmation serveur...\n");
    if(Rio_readnb(&rio, buf, 3)>0){
        if(strcmp(buf,"ok")!=0){
            printf("Erreur: ce fichier n'existe pas.\n");
            return;
        }
        printf("Serveur ok. \n");
    }    
    else{
        fprintf(stderr,"Serveur déconnecté.\n");
        exit(1);
    }

    //Reception taille fichier à télécharger
    int tailleTotale;
    printf("Attente taille du fichier...\n");
    Rio_readnb(&rio, &tailleTotale, sizeof(int));
    printf("Téléchargement de %s (%d octets).\n",filePath,tailleTotale);
        
    //Reception date de dernière modification du fichier à télécharger
    printf("Attente date fichier...\n");
    time_t dateModificationServeur;
    Rio_readnb(&rio, &dateModificationServeur, sizeof(time_t));
    strftime(date, 20, "%Y-%m-%d %H:%M:%S", localtime(&dateModificationServeur));
    printf("Dernière modification : %s\n",date);

    /*
    *Ouverture ou création fichier:
    *    Si le client possède déjà un fichier de même nom:
    *        -si le fichier du serveur est plus récent que le fichier du client, on réécrit tout le fichier
    *        -sinon on écrit à la suite du fichier existant
    *    Sinon
    *        -on crée un nouveau fichier
    */
    int departTelechargement = 0; //Numéro de l'octet à partir duquel commencer à télécharger le fichier
    struct stat infoFichierClient; 
    if (stat(filePath, &infoFichierClient) == 0){
        //Le client possède déjà le fichier
        printf("Fichier déjà présent chez le client.\n");
        time_t dateModificationClient = infoFichierClient.st_mtime;
        if(difftime(dateModificationServeur,dateModificationClient)>0){
            //Le fichier du serveur est plus récent que le fichier du client => on retélécharge complètement le fichier
            printf("Fichier plus récent trouvé, mise à jour complète...\n");
            fd = Open(filePath,O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
            departTelechargement = 0;
        }
        else{
            printf("Fichier moins récent trouvé, mise à jour partielle...\n");
            fd = Open(filePath,O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
            departTelechargement = infoFichierClient.st_size;
        }
    }
    else{
        //Le client ne possède pas le fichier => on le crée
        printf("Fichier absent chez le client, création...\n");
        fd = Open(filePath,O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        departTelechargement = 0;
    }

    int nbOctetsATelecharger = tailleTotale-departTelechargement;

    //Envoie du numéro de l'octet à partir duquel commencer à télécharger le fichier
    Rio_writen(clientfd, &departTelechargement, sizeof(int));

    if(nbOctetsATelecharger>0)
        printf("%d octets vont être téléchargés.\n",nbOctetsATelecharger);
    
    //temps début transfert
    t1 = time(NULL);
    int m,totalBytes=0,bufferSize;
    bufferSize = MAXLINE;//Nombre d'octets à lire à chaque Read
    while(bufferSize>nbOctetsATelecharger-totalBytes) bufferSize/=2;//On ne liera jamais + que le fichier (permet de ne pas avoir de Read bloquant)
    //lecture du fichier envoyé par le serveur, on s'arrête quand on a lu le bon nombre d'octets ou que le serveur crash
    while(totalBytes<nbOctetsATelecharger && ((m=Rio_readnb(&rio, buf, bufferSize)) > 0)){
        if(m<0){
        fprintf(stderr,"Serveur déconnecté.\n");
        exit(1);
        }
        Rio_writen(fd, buf, m);//écriture dans le fichier du client
        totalBytes+=m;//Nombre d'octets lu courrant
        //printf("(%d/%d)\n",totalBytes,nbOctetsATelecharger);
        while(bufferSize>nbOctetsATelecharger-totalBytes) bufferSize/=2;//On ne liera jamais + que le fichier (permet de ne pas avoir de Read bloquant)
    }

    //temps fin transfert
    t2 = time(NULL);
    if(nbOctetsATelecharger!=totalBytes){
        printf("Transfer incomplete.\n");
    }
    else{
        if(tailleTotale-departTelechargement==0)
        printf("Rien à télécharger.\n");
        else
        printf("Transfer successfully complete.\n");
    }
    printf("%d bytes received in %ld seconds (%d Kbytes/s)\n",totalBytes,t2-t1,totalBytes/max(1,t2-t1));
    Close(fd);
}

int main(int argc, char **argv)
{
    int clientfd, port;
    char *host;
    char buf[MAXLINE] = {'\0'};
    rio_t rio;
    char* request;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    //port = atoi(argv[2]);
    port = 2121;

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, port);
    
    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */
    printf("client connected to server OS\n"); 
    
    Rio_readinitb(&rio, clientfd);
    printf("> ");

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf));
        //extraction de la comande.
        request = strtok(buf," \n"); //here's the command.
        printf("command : %s \n",request);
        if(strcmp(request,"get") == 0){
            request = strtok(NULL," \n");//here the file if the cmd was get.
            printf("file : %s \n",request);
            client_get(clientfd,rio,request);
        }
        else{
            printf("commande : %s unknown\n",request);
        }

    }
    Close(clientfd);
    exit(0);
}
