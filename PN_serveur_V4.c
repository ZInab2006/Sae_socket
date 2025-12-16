#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 5000
#define PORT_GAME 6000
#define LG_MESSAGE 512

int main(int argc, char *argv[]){
    
    // Declarartion des variables
    //===========================
    int socketEcoute;
    struct sockaddr_in adresseServeur;
    socklen_t longueurAdresse;

    int socketDialogueClient1;
    struct sockaddr_in adresseClient1;

    int socketDialogueClient2;
    struct sockaddr_in adresseClient2;

    char buffer[LG_MESSAGE];


    // Création socket d'écoute
    //==========================
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if (socketEcoute < 0) {
        perror("socket");
        exit(-1);
    }
    printf("Socket créée avec succès ! (%d)\n", socketEcoute);


    // Option pour réutiliser l'adresse immédiatement
    //================================================
    int opt = 1;
    setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));


    // Remplissage adresseServeur
    //===========================
    longueurAdresse = sizeof(adresseServeur);
    memset(&adresseServeur, 0x00, longueurAdresse);
    adresseServeur.sin_family = PF_INET;
    adresseServeur.sin_addr.s_addr = htonl(INADDR_ANY);
    adresseServeur.sin_port = htons(PORT);


    // Bind socket - Adresse
    //=======================
    if ((bind(socketEcoute, (struct sockaddr *)&adresseServeur, longueurAdresse)) < 0) {
        perror("bind");
        exit(-2);
    }
    printf("Socket attachée avec succès !\n");


    // Listen
    //=======
    if (listen(socketEcoute, 10) < 0) {
        perror("listen");
        exit(-3);
    }
    printf("Socket placée en écoute passive sur le port %d...\n", PORT);



    // Boucle principale en attente de client
    //=======================================
    while(1){
        
        char ip_client1[16];
        int portClient1;
        
        char ip_client2[16];
        int portClient2;

        //      INITIALISATION SOCKET CONNEXION CLIENT
        //===================================================
        printf("\n=== Attente d'une demande de connexion (quitter avec Ctrl-C) ===\n");


        // Acceptation Client 1
        //=====================
        socketDialogueClient1 = accept(socketEcoute, (struct sockaddr *) &adresseClient1, &longueurAdresse);
        if (socketDialogueClient1 < 0) {
            perror("accept client1");
            continue;
        }
        printf("\n>>> Connexion Client1 acceptée depuis %s:%d\n", 
               inet_ntoa(adresseClient1.sin_addr),
               ntohs(adresseClient1.sin_port));


        // recuperation des ip et port client1
        //====================================
        strcpy(ip_client1, inet_ntoa(adresseClient1.sin_addr));
        portClient1 = ntohs(adresseClient1.sin_port);
        
        
        // Choisir le Client qui fera office de serveur
        //=============================================
        memset(buffer, 0, LG_MESSAGE);
        snprintf(buffer, LG_MESSAGE, "SERVEUR %d", PORT_GAME); // Envoi du port qui va être utilisé pour le jeu
        send(socketDialogueClient1, buffer, strlen(buffer) + 1, 0);
        printf("Message d'identification client 1\n");
        
        
        // Acceptation Client 2
        //=====================
        printf("En attente d'une deuxieme connexion...\n");
        socketDialogueClient2 = accept(socketEcoute, (struct sockaddr *) &adresseClient2, &longueurAdresse);
        if (socketDialogueClient2 < 0) {
            perror("accept");
            continue;
        }
        printf("\n>>> Connexion Client2 acceptée depuis %s:%d\n", inet_ntoa(adresseClient2.sin_addr), ntohs(adresseClient2.sin_port));
        

        // recuperation des ip et port client2
        //====================================
        strcpy(ip_client2, inet_ntoa(adresseClient2.sin_addr));
        portClient2 = ntohs(adresseClient2.sin_port);

        
        // Lancer la partie sur le client 2
        //=================================
        memset(buffer, 0, LG_MESSAGE);
        snprintf(buffer, LG_MESSAGE, "CLIENT %s %d", ip_client1, PORT_GAME); // envoi de l'adresse ip du client qui va servir de serveur et du port qui va être utilisé pour le jeu
        send(socketDialogueClient2, buffer, strlen(buffer) + 1, 0);
        printf("Message d'identification client 1\n");
        sleep(1);


        // Fermeture des sockets - les clients sont autonomes
        //===================================================
        close(socketDialogueClient1);
        close(socketDialogueClient2);
    }

    // Fermer socket écoute (jamais atteint)
    close(socketEcoute);
    return 0;
}
