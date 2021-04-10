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
    time_t dateModifServeur;
    Rio_readnb(&rio, &dateModifServeur, sizeof(time_t));
    strftime(date, 20, "%Y-%m-%d %H:%M:%S", localtime(&dateModifServeur));
    printf("Dernière modification : %s\n",date);


    /*
    On vérifie si le client possède ou non le fichier
        si oui et que le fichier serveur est plus récent on redl l'ensemble du fichier
        sinon on met à jour le fichier existant en écrivant ce qu'il manque
        si le fichier n'existe pas on le crée.
    */
   
    int startByte = 0; //Numéro de l'octet à partir duquel commencer à télécharger le fichier
    struct stat info; 
    if (stat(filePath, &info) == 0){
        //Le client possède déjà le fichier
        printf("Fichier déjà présent chez le client.\n");
        time_t dateModifClient = info.st_mtime; //extraction date modif client grâce à stat
        if(difftime(dateModifServeur,dateModifClient)>0){
            //Le fichier du serveur est plus récent que le fichier du client => on retélécharge complètement le fichier
            printf("Fichier plus récent trouvé, mise à jour complète...\n");
            fd = Open(filePath,O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
            startByte = 0;
        }
        else{
            printf("Fichier moins récent trouvé, mise à jour partielle...\n");
            fd = Open(filePath,O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
            startByte = info.st_size;
        }
    }
    else{
        //Le client ne possède pas le fichier => on le crée
        printf("Fichier absent chez le client, création...\n");
        fd = Open(filePath,O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        startByte = 0;
    }

    int TailleDL = tailleTotale-startByte;

    //Envoie du numéro de l'octet à partir duquel commencer à télécharger le fichier
    Rio_writen(clientfd, &startByte, sizeof(int));

    if(TailleDL>0){
        printf("%d octets vont être téléchargés.\n",TailleDL);
    }
    
    //temps début transfert
    t1 = time(NULL);
    int n,SommeOctetDL=0,bufferSize;
    bufferSize = MAXLINE;//Nombre d'octets à lire à chaque Read
    while(bufferSize>TailleDL-SommeOctetDL)bufferSize/=2;//On divise la taille du buffer pour ne par lire trop (notemment en fin de fichier)
    //lecture du fichier envoyé par le serveur et écriture dans celui crée/modifié chez le client. Tant que la tailleDL n'est pas atteinte (et qu'on).
    while(SommeOctetDL<TailleDL && ((n=Rio_readnb(&rio, buf, bufferSize)) > 0)){
        if(n<0){
            fprintf(stderr,"Serveur déconnecté.\n");
            exit(1);
        }
        Rio_writen(fd, buf, n);//écriture dans le fichier du client
        SommeOctetDL+=n;//Incrémentation du nombre d'octet déjà transféré.
        printProgress((SommeOctetDL*100)/TailleDL);
        while(bufferSize>TailleDL-SommeOctetDL) bufferSize/=2;
        usleep(50000);
    }

    //temps fin transfert
    t2 = time(NULL);
    if(TailleDL!=SommeOctetDL){
        printf("\nTransfer incomplete.\n");
    }
    else{
        if(tailleTotale-startByte==0){
            printf("\nRien à télécharger.\n");
        }
        else{
            printf("\nTransfer successfully complete.\n");
        }
    }
    printf("%d bytes received in %ld seconds (%d Kbytes/s)\n",SommeOctetDL,t2-t1,SommeOctetDL/max(1,t2-t1));
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
    port = 2198;

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

        if(strcmp(buf,"\n")!=0){
            Rio_writen(clientfd, buf, strlen(buf));
             //extraction de la comande.
            request = strtok(buf," \n"); //here's the command.

            if(strcmp(request,"get") == 0){
            request = strtok(NULL," \n");//here the file if the cmd was get.
            client_get(clientfd,rio,request);
            }
            else if(strcmp(buf,"bye")==0){
                exit(0);
            }
            else{
                printf("%s : command unknown\n",request);
            }
        }
        printf("> ");
    }
    Close(clientfd);
    exit(0);
}
