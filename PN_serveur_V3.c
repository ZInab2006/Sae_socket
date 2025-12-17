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
#define LG_MESSAGE 512
#define MAX_ERREURS 6
#define MOT_SECRET "PENDU"
#define MAX_MOT 50
#define MAX_PARTIES 10

/* 
Joueur 1 : fait deviner (entre le mot)
Joueur 2 : devine (propose des lettres)

- CODES
    * 2001 : client & serveur, si le serveur reçoit ce code (à l'inverse il demande au client 1 de choisir un mot), code 2004 envoyé -> la partie commence 
    * 2002 : le joueur 2 reçoit le mot, il peut commencer à deviner 
    * 2003 : le joueur 2 propose une lettre
    * 2004 : le joueur 1 valide ou non la lettre proposée par le joueur
    * 2005 : le joueur 2 reçoit la validation ou non de sa lettre
    * 2006 : le joueur 2 reçoit les données de la partie (mot affiché, nombre d'erreurs, statut)
    * 3001 : message affichant une information, elle ne declenche aucune action d'un des clients
*/

#define JOUEUR_1_ENTRE_MOT 2001 
#define JOUEUR_2_TAILLE_MOT_ET_PEUT_JOUER 2002
#define JOUEUR_2_PROPOSE_LETTRE_OU_MOT 2003
#define JOUEUR_1_VALIDE_OU_NON 2004
#define JOUEUR_2_RECOIT_VALIDE_OU_NON 2005
#define JOUEUR_2_DONNEES_PARTIE 2006
#define MESSAGE 3001 

/*
 * Fonction permettant l'envoi de message avec format : code:données
 * Elle permet la redondance de fonctions lourdes, rend le code plus clair et plus facilement modifiable
*/
void emit(const int code, const char* buffer, int socket, int client) {
    char message[LG_MESSAGE];
    sprintf(message, "%d:%s", code, buffer);
    send(socket, message, strlen(message), 0);
    printf("Envoyé au client %d : '%s'\n", client, buffer);
}

/*
 * Fonction permettant la réception et le traitement d'un message
*/
void traiter_message(char* messageRecu, int socket_client) {
    memset(messageRecu, 0, LG_MESSAGE);
    int lus = recv(socket_client, messageRecu, LG_MESSAGE-1, 0);
    if (lus <= 0) {
        printf("Le client %d s'est déconnecté.\n", socket_client);
        close(socket_client);
        exit(-7);
    }

    messageRecu[lus] = '\0';
}


/**
 * Serveur V2 - Fait transiter les messages entre 2 clients au minimum, sinon aucune partie de créée.
 * Client 1 : fait deviner (entre le mot)
 * Client 2 : devine (propose des lettres)
 */
int main(int argc, char *argv[]){
    int socketEcoute;
    int socket_client_1, socket_client_2;
    struct sockaddr_in adresseServeur;
    socklen_t longueurAdresse;
    struct sockaddr_in adresseClient;
    char messageRecu[LG_MESSAGE];
    int lus;


    // Création socket d'écoute
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if (socketEcoute < 0) {
        perror("socket");
        exit(-1);
    }
    printf("Socket créée avec succès ! (%d)\n", socketEcoute);

    // Option pour réutiliser le port
    int opt = 1;
    setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configuration adresse serveur
    longueurAdresse = sizeof(adresseServeur);
    memset(&adresseServeur, 0x00, longueurAdresse);
    adresseServeur.sin_family = PF_INET;
    adresseServeur.sin_addr.s_addr = htonl(INADDR_ANY);
    adresseServeur.sin_port = htons(PORT);

    // Bind
    if ((bind(socketEcoute, (struct sockaddr *)&adresseServeur, longueurAdresse)) < 0) {
        perror("bind");
        exit(-2);
    }
    printf("Socket attachée avec succès !\n");

    // Listen
    if (listen(socketEcoute, 5) < 0) {
        perror("listen");
        exit(-3);
    }
    printf("Socket placée en écoute passive sur le port %d...\n", PORT);

    // Boucle principale
    while (1) {
        char mot_secret[MAX_MOT];
        
        // Attente client 1
        printf("\n=== Attente du premier client ===\n");

        longueurAdresse = sizeof(adresseClient);
        socket_client_1 = accept(socketEcoute, (struct sockaddr *) &adresseClient, &longueurAdresse);
        
        if (socket_client_1 < 0) {
            perror("Erreur accept client 1");
            continue;
        }

        printf(">>> Client 1 connecté depuis %s:%d\n", inet_ntoa(adresseClient.sin_addr), ntohs(adresseClient.sin_port));
        emit(MESSAGE, "Bienvenue joueur 1 ! Attendez qu'un autre joueur arrive...", socket_client_1, 1);
		

        // Attente client 2
        printf("\n=== Attente du deuxième client ===\n");

        longueurAdresse = sizeof(adresseClient);
        socket_client_2 = accept(socketEcoute, (struct sockaddr *) &adresseClient, &longueurAdresse);
        if (socket_client_2 < 0) {
            perror("Erreur accept client 2");
            close(socket_client_1);
            continue;
        }

        // Démarrage -> Initiation d'un fork pour rendre le serveur multijoueur (multi-processus)
        pid_t partie = fork();

        if(partie == 0){
            int reponse_code;
            char reponse_message[LG_MESSAGE];

            printf(">>> Client 2 connecté depuis %s:%d\n", inet_ntoa(adresseClient.sin_addr), ntohs(adresseClient.sin_port));
            emit(MESSAGE, "Bienvenue Client 2 !\nLe jeu va commencer. Attendez que l'autre joueur choissise un mot à deviner...\n", socket_client_2, 2);

            emit(JOUEUR_1_ENTRE_MOT, "", socket_client_1, 1);

            // Recevoir le mot
            traiter_message(messageRecu, socket_client_1);
            printf("Le mot entré par le client 1 : %s\n", messageRecu);

            char longueur_mot_string[MAX_MOT];
            strncpy(mot_secret, messageRecu, MAX_MOT-1);
            mot_secret[MAX_MOT-1] = '\0';
            snprintf(longueur_mot_string, sizeof(longueur_mot_string), "%ld", strlen(mot_secret));

            emit(JOUEUR_2_TAILLE_MOT_ET_PEUT_JOUER, longueur_mot_string, socket_client_2, 2);
            emit(MESSAGE, "Le joueur 2 devine...", socket_client_1, 2);

            // Boucle de transit des messages
            printf("\n=== Début de la partie ===\n");
            int partie_en_cours = 1;
            char status[32]; // erreur, gagne, perdu
            char mot_affiche[LG_MESSAGE];
            int nb_erreurs;

            while (partie_en_cours) {
                // Recevoir lettre du client 2
                traiter_message(messageRecu, socket_client_2);
                
                printf("Reçu du client 2 (lettre) : %s → Transite au client 1\n", messageRecu);

                emit(JOUEUR_2_PROPOSE_LETTRE_OU_MOT, messageRecu, socket_client_1, 1);

                // Recevoir réponse du client 1
                traiter_message(messageRecu, socket_client_1);
                printf("Reçu du client 1 (réponse) : %s → Transite au client 2\n", messageRecu);

                // Vérifier fin de partie
                if (strstr(messageRecu, "gagne") != NULL || strstr(messageRecu, "perdu") != NULL) {
                    printf(">>> Partie terminée !\n");
                    partie_en_cours = 0;
                }

                emit(JOUEUR_2_DONNEES_PARTIE, messageRecu, socket_client_2, 2);
            }

            // Fermeture
            close(socket_client_1);
            close(socket_client_2);
            
            printf("\n=== Fin de la partie ===\n");
            printf("Le mot à deviner était : %s\n", mot_secret);
            printf("Les connexion ont été fermées.\n\n");
		} else if (partie > 0) {
            // ========= PARENT =========
            close(socket_client_1);
            close(socket_client_2);
            
            // Le parent retourne au accept()
        } else {
            perror("fork");
        }
    }

    close(socketEcoute);
    return 0;
}
