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
#define MAX_MOT 50
#define ERREURS_MAX 6

// Fonction pour afficher le pendu
void afficher_pendu(int nb_erreurs) {
    printf("\n");
    switch(nb_erreurs) {
        case 0:
            printf("  +---+\n");
            printf("  |   |\n");
            printf("      |\n");
            printf("      |\n");
            printf("      |\n");
            printf("      |\n");
            printf("=========\n");
            break;
        case 1:
            printf("  +---+\n");
            printf("  |   |\n");
            printf("  O   |\n");
            printf("      |\n");
            printf("      |\n");
            printf("      |\n");
            printf("=========\n");
            break;
        case 2:
            printf("  +---+\n");
            printf("  |   |\n");
            printf("  O   |\n");
            printf("  |   |\n");
            printf("      |\n");
            printf("      |\n");
            printf("=========\n");
            break;
        case 3:
            printf("  +---+\n");
            printf("  |   |\n");
            printf("  O   |\n");
            printf(" /|   |\n");
            printf("      |\n");
            printf("      |\n");
            printf("=========\n");
            break;
        case 4:
            printf("  +---+\n");
            printf("  |   |\n");
            printf("  O   |\n");
            printf(" /|\\  |\n");
            printf("      |\n");
            printf("      |\n");
            printf("=========\n");
            break;
        case 5:
            printf("  +---+\n");
            printf("  |   |\n");
            printf("  O   |\n");
            printf(" /|\\  |\n");
            printf(" /    |\n");
            printf("      |\n");
            printf("=========\n");
            break;
        case 6:
            printf("  +---+\n");
            printf("  |   |\n");
            printf("  O   |\n");
            printf(" /|\\  |\n");
            printf(" / \\  |\n");
            printf("      |\n");
            printf("=========\n");
            printf("  PERDU !\n");
            break;
        default:
            break;
    }
    printf("\n");
}



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

        printf("\n>>> Connexion P2P établie ! Vous êtes le joueur qui fait deviner.\n\n");

        // Variables pour le jeu
        char mot_secret[MAX_MOT];
        char mot_affiche[MAX_MOT];
        char lettres_deja_testees[26] = {0};
        char messageRecu[LG_MESSAGE];
        int lus;
        int nb_erreurs = 0;

        // Demander le mot secret
        printf("Veuillez entrer un mot à deviner : ");
        scanf("%s", mot_secret);

        // Conversion en majuscules
        for(int i = 0; mot_secret[i]; i++) {
            mot_secret[i] = toupper((unsigned char)mot_secret[i]);
        }

        // Initialisation avec des tirets
        int longueur_mot = strlen(mot_secret);
        for(int i=0; i<longueur_mot; i++)
            mot_affiche[i] = '_';
        mot_affiche[longueur_mot] = '\0';

        // Envoi de "start x" au client qui devine
        memset(buffer, 0, LG_MESSAGE);
        snprintf(buffer, LG_MESSAGE, "start %d", longueur_mot);
        send(socketP2P, buffer, strlen(buffer) + 1, 0);
        printf("Mot secret enregistré : %s\n", mot_secret);
        printf("En attente des propositions de lettres...\n\n");

        // Boucle de jeu - Réception des lettres et envoi des réponses
        while(1){
            memset(messageRecu, 0, LG_MESSAGE);
            lus = recv(socketP2P, messageRecu, LG_MESSAGE-1, 0);
            if(lus <= 0) break;
            
            messageRecu[lus] = '\0';
            char lettre = toupper((unsigned char)messageRecu[0]);
            
            // Vérification lettre valide
            if(!isalpha((unsigned char)lettre)) {
                snprintf(buffer, LG_MESSAGE, "erreur %s %d", mot_affiche, nb_erreurs);
                send(socketP2P, buffer, strlen(buffer) + 1, 0);
                continue;
            }
            
            // Vérification si déjà testée
            if(lettres_deja_testees[lettre - 'A']){
                snprintf(buffer, LG_MESSAGE, "deja %s %d", mot_affiche, nb_erreurs);
                send(socketP2P, buffer, strlen(buffer) + 1, 0);
                continue;
            }
            lettres_deja_testees[lettre - 'A'] = 1;
            
            // Recherche de la lettre dans le mot
            int trouve = 0;
            for(int i=0; i<longueur_mot; i++){
                if(mot_secret[i] == lettre){
                    mot_affiche[i] = lettre;
                    trouve = 1;
                }
            }
            
            if(!trouve)
                nb_erreurs++;
            
            // Détermination du statut
            char status[20];
            int partie_terminee = 0;
            
            // Vérifier si mot complet
            int mot_complet = 1;
            for(int i=0; i<longueur_mot; i++) {
                if(mot_affiche[i] == '_') {
                    mot_complet = 0;
                    break;
                }
            }
            
            if(mot_complet) {
                strcpy(status, "gagne");
                partie_terminee = 1;
            } else if(nb_erreurs >= ERREURS_MAX) {
                strcpy(status, "perdu");
                partie_terminee = 1;
            } else {
                strcpy(status, trouve ? "oui" : "non");
            }
            
            // Envoi de la réponse
            memset(buffer, 0, LG_MESSAGE);
            if(partie_terminee) {
                snprintf(buffer, LG_MESSAGE, "%s %s %d", status, 
                         strcmp(status, "perdu") == 0 ? mot_secret : mot_affiche, 
                         nb_erreurs);
            } else {
                snprintf(buffer, LG_MESSAGE, "%s %s %d", status, mot_affiche, nb_erreurs);
            }
            
            send(socketP2P, buffer, strlen(buffer) + 1, 0);
            
            if(partie_terminee) {
                if(strcmp(status, "gagne") == 0) {
                    printf("\n>>> Le joueur a gagné ! Le mot était : %s\n", mot_affiche);
                } else {
                    printf("\n>>> Le joueur a perdu ! Le mot était : %s\n", mot_secret);
                }
                break;
            }
        }
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
        printf(">>> Connexion P2P réussie ! Vous êtes le joueur qui devine.\n\n");

        // Variables pour le jeu
        char status[20];
        char mot_decouvert[LG_MESSAGE];
        int nb_erreurs;
        int longueur_mot;

        // Réception "start x"
        memset(buffer, 0, LG_MESSAGE);
        nb = recv(socketP2P, buffer, LG_MESSAGE, 0);
        if (nb <= 0) {
            perror("Erreur réception");
            close(socketP2P);
            exit(-4);
        }
        buffer[nb] = '\0';
        
        if (sscanf(buffer, "start %d", &longueur_mot) != 1) {
            printf("Message inattendu : %s\n", buffer);
            close(socketP2P);
            exit(-5);
        }
        
        printf("\n===========================================\n");
        printf("    BIENVENUE AU JEU DU PENDU !\n");
        printf("===========================================\n");
        printf("Mot de %d lettres à deviner\n", longueur_mot);
        printf("Vous avez droit à %d erreurs maximum\n", ERREURS_MAX);
        printf("===========================================\n\n");

        // Boucle de jeu - Proposition de lettres
        while (1) {
            char lettre;
            printf("\nEntrez une lettre : ");
            if (scanf(" %c", &lettre) != 1) {
                printf("Erreur de saisie\n");
                continue;
            }
            
            // Nettoyage buffer
            int c;
            int chars_ignores = 0;
            while ((c = getchar()) != '\n' && c != EOF) {
                chars_ignores++;
            }
            if (chars_ignores > 0) {
                printf("⚠ Attention : seule la première lettre a été prise en compte.\n");
            }
            
            lettre = toupper(lettre);
            
            if (!isalpha(lettre)) {
                printf("Veuillez entrer une lettre valide (A-Z)\n");
                continue;
            }
            
            // Envoi de la lettre
            memset(buffer, 0, LG_MESSAGE);
            sprintf(buffer, "%c", lettre);
            send(socketP2P, buffer, strlen(buffer) + 1, 0);
            
            // Réception réponse
            memset(buffer, 0, LG_MESSAGE);
            nb = recv(socketP2P, buffer, LG_MESSAGE, 0);
            if (nb <= 0) {
                perror("Erreur réception");
                close(socketP2P);
                exit(-7);
            }
            buffer[nb] = '\0';
            
            // Parsing
            if (sscanf(buffer, "%s %s %d", status, mot_decouvert, &nb_erreurs) != 3) {
                printf("Réponse inattendue : %s\n", buffer);
                continue;
            }
            
            // Affichage
            printf("\n");
            printf("Mot : ");
            for (int i = 0; i < strlen(mot_decouvert); i++) {
                printf("%c ", mot_decouvert[i]);
            }
            printf("\n");
            printf("Erreurs : %d/%d\n", nb_erreurs, ERREURS_MAX);
            
            afficher_pendu(nb_erreurs);
            
            if (strcmp(status, "oui") == 0) {
                printf("✓ Bonne lettre !\n");
            } else if (strcmp(status, "non") == 0) {
                printf("✗ Mauvaise lettre.\n");
            } else if (strcmp(status, "deja") == 0) {
                printf("⚠ Lettre déjà choisie. Choisissez-en une autre.\n");
            } else if (strcmp(status, "erreur") == 0) {
                printf("⚠ Caractère invalide. Entrez une lettre (A-Z).\n");
            }
            
            // Fin de partie
            if (strcmp(status, "gagne") == 0) {
                printf("\n");
                printf("*******************************************\n");
                printf("  FÉLICITATIONS ! VOUS AVEZ GAGNÉ !\n");
                printf("  Le mot était : %s\n", mot_decouvert);
                printf("  Nombre d'erreurs : %d\n", nb_erreurs);
                printf("*******************************************\n");
                break;
            } else if (strcmp(status, "perdu") == 0) {
                printf("\n");
                printf("*******************************************\n");
                printf("  DOMMAGE ! VOUS AVEZ PERDU !\n");
                printf("  Le mot était : %s\n", mot_decouvert);
                printf("*******************************************\n");
                break;
            }
        }
    }
    
    //Fermeture du socket P2P
    //=======================
    close(socketP2P);
    printf("\nConnexion fermée. Au revoir !\n");
    return 0;
}
