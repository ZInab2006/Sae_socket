#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define LG_MESSAGE 512
#define CLEAR_SCREEN "\033[2J\033[H"
#define ERREURS_MAX 6



int main(int argc, char *argv[]){
    //=======================================================
    //              PARTIE DECLARATIVE
    //=======================================================
    int descripteurSocket;
    struct sockaddr_in adresseServeur;
    socklen_t longueurAdresse;
    
    char ip_dest[16];
    int port_dest;
    
    char buffer[LG_MESSAGE];
    int nb;

    char ip_dest_game[16];
    int port_dest_game;

    char entete[32];
    char message[LG_MESSAGE];

    //=======================================================
    //      CONNEXION AU SERVEUR PRINCIPAL
    //=======================================================
    // Récupération IP et port
    //========================
    if (argc < 3) {
        printf("USAGE : %s ip port\n", argv[0]);
        printf("Exemple : %s 127.0.0.1 5000\n", argv[0]);
        exit(-1);
    }
    strncpy(ip_dest, argv[1], 15);
    ip_dest[15] = '\0';
    sscanf(argv[2], "%d", &port_dest);

    // Création socket
    //================
    descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (descripteurSocket < 0) {
        perror("Erreur en création de la socket");
        exit(-1);
    }
    printf("Socket créée! (%d)\n", descripteurSocket);

    // Remplissage adresseServeur
    //===========================
    longueurAdresse = sizeof(adresseServeur);
    memset(&adresseServeur, 0x00, longueurAdresse);
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_port = htons(port_dest);
    if (inet_aton(ip_dest, &adresseServeur.sin_addr) == 0) {
        printf("Adresse IP invalide : %s\n", ip_dest);
        close(descripteurSocket);
        exit(-2);
    }

    // Connexion
    //==========
    printf("Tentative de connexion à %s:%d...\n", ip_dest, port_dest);
    if ((connect(descripteurSocket, (struct sockaddr *) &adresseServeur, longueurAdresse)) == -1) {
        perror("Erreur de connexion avec le serveur distant");
        close(descripteurSocket);
        exit(-3);
    }
    printf(">>> Connexion au serveur %s:%d réussie!\n\n", ip_dest, port_dest);


    // Recuperation du port de com
    //==============================
    memset(buffer, 0, LG_MESSAGE);
    nb = recv(descripteurSocket, buffer, LG_MESSAGE, 0);
    if (nb <= 0) {
        perror("Erreur réception");
        close(descripteurSocket);
        exit(-4);
    }
    buffer[nb] = '\0';
    printf("message recu : %s\n", buffer);

    // Récupération des informations nécessaires pour la connexion P2P
    // ip serveur + port + Role de chacun des clients
    //================================================================
    if(sscanf(buffer,"SERVEUR %d", &port_dest_game) == 1){
        strcpy(entete, "SERVEUR");
    }
    else if(sscanf(buffer, "CLIENT %s %d", ip_dest_game, &port_dest_game) == 2){
        strcpy(entete, "CLIENT");
    }
    else{
        printf("erreur de format de message.\n");
        exit(-5);
    }
    

    // Fermer le socket avec serveur de départ
    //========================================
    close(descripteurSocket);


    // Nouvelles sockets
    //==================
    int socketP2P;
    struct sockaddr_in adresseClient;
    socklen_t longueurAdresseClient;

    int socketEcouteNewServeur;
    struct sockaddr_in adresseNewServeur;
    socklen_t longueurAdresseNewServeur;

    //=======================================================================
    //                       Mode SERVEUR
    //=======================================================================
    if (strcmp(entete, "SERVEUR") == 0){
                
        // Creation socket Ecoute
        //=======================
        socketEcouteNewServeur = socket(AF_INET, SOCK_STREAM, 0);
        if (socketEcouteNewServeur < 0) {
        perror("socket");
        exit(-1);
        }

        int opt = 1;
        setsockopt(socketEcouteNewServeur, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        //Bind avec socket P2P
        //====================
        longueurAdresseNewServeur = sizeof(adresseNewServeur);
        memset(&adresseNewServeur, 0x00, longueurAdresseNewServeur);
        adresseNewServeur.sin_family = PF_INET;
        adresseNewServeur.sin_addr.s_addr = htonl(INADDR_ANY);
        adresseNewServeur.sin_port = htons(port_dest_game);

        if ((bind(socketEcouteNewServeur, (struct sockaddr *)&adresseNewServeur, longueurAdresseNewServeur)) < 0) {
            perror("bind");
            exit(-2);
        }
        printf("Socket attachée avec succès !\n");

        // Listen - En attente du CLIENT joueur
        //=====================================
        if (listen(socketEcouteNewServeur, 1) < 0) {
            perror("listen");
            exit(-3);
        }
        printf("Socket placée en écoute passive sur le port %d...\n", port_dest_game);

        // Acceptation Client 1
        //=====================
        longueurAdresseClient = sizeof(adresseClient);
        socketP2P = accept(socketEcouteNewServeur, (struct sockaddr *) &adresseClient, &longueurAdresseClient);
        if (socketP2P < 0) {
            perror("accept client1");
        }

        // Fermeture de la socket d'écoute
        //================================
        close(socketEcouteNewServeur);




        // Boucle de jeu
        // partie du code sur le joueur qui fait deviner le mot




    }
    //=======================================================================
    //                       Mode CLIENT
    //=======================================================================
    else{

        // Création socket
        //================
        socketP2P = socket(AF_INET, SOCK_STREAM, 0);
        if (socketP2P < 0) {
            perror("Erreur de création de la socket");
            exit(-1);
        }
        printf("Socket créée! (%d)\n", socketP2P);

        // Remplissage adresseServeur
        //===========================
        longueurAdresseNewServeur = sizeof(adresseNewServeur);
        memset(&adresseNewServeur, 0x00, longueurAdresseNewServeur);
        adresseNewServeur.sin_family = AF_INET;
        adresseNewServeur.sin_port = htons(port_dest_game);
        if (inet_aton(ip_dest_game, &adresseNewServeur.sin_addr) == 0) {
            printf("Adresse IP invalide : %s\n", ip_dest_game);
            close(socketP2P);
            exit(-2);
        }

        // Connexion
        //==========
        printf("Tentative de connexion à %s:%d...\n", ip_dest_game, port_dest_game);
        if ((connect(socketP2P, (struct sockaddr *) &adresseNewServeur, longueurAdresseNewServeur)) == -1) {
            perror("Erreur de connexion avec le serveur distant");
            close(socketP2P);
            exit(-3);
        }
        printf(">>> Connexion au serveur %s:%d réussie!\n\n", ip_dest_game, port_dest_game);




        // Boucle de jeu
        // partie sur le joueur qui devine le mot



    }
    
    //Fermeture du socket P2P
    //=======================
    close(socketP2P);
    printf("\nConnexion fermée. Au revoir !\n");
    return 0;
}
