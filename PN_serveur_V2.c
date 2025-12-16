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

/**
 * Serveur V2 - Fait transiter les messages entre 2 clients
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
        char buffer[LG_MESSAGE];
        char mot_secret[MAX_MOT];
        int longueur_mot;
        
        // Attente client 1
        printf("\n=== Attente du premier client ===\n");

        longueurAdresse = sizeof(adresseClient);
        socket_client_1 = accept(socketEcoute, (struct sockaddr *) &adresseClient, &longueurAdresse);
        
        if (socket_client_1 < 0) {
            perror("Erreur accept client 1");
            continue;
        }
        printf(">>> Client 1 connecté depuis %s:%d\n", inet_ntoa(adresseClient.sin_addr), ntohs(adresseClient.sin_port));

        // Demander le mot
        strcpy(buffer, "Veuillez entrer un mot à deviner : ");
        send(socket_client_1, buffer, strlen(buffer) + 1, 0);
        printf("Envoyé au client 1 : %s\n", buffer);

        // Recevoir le mot
        memset(messageRecu, 0, LG_MESSAGE);
        lus = recv(socket_client_1, messageRecu, LG_MESSAGE-1, 0);
        if (lus <= 0) {
            printf("Le client 1 s'est déconnecté.\n");
            close(socket_client_1);
            continue;
        }
        messageRecu[lus] = '\0';
        printf("Le mot entré par le client 1 : %s\n", messageRecu);

        strncpy(mot_secret, messageRecu, MAX_MOT-1);
        mot_secret[MAX_MOT-1] = '\0';
        longueur_mot = strlen(messageRecu);

        // Attente client 2
        printf("\n=== Attente du deuxième client ===\n");

        longueurAdresse = sizeof(adresseClient);
        socket_client_2 = accept(socketEcoute, (struct sockaddr *) &adresseClient, &longueurAdresse);
        if (socket_client_2 < 0) {
            perror("Erreur accept client 2");
            close(socket_client_1);
            continue;
        }
        printf(">>> Client 2 connecté depuis %s:%d\n", inet_ntoa(adresseClient.sin_addr), ntohs(adresseClient.sin_port));

        // Envoi "start x"
        sprintf(buffer, "start %d", longueur_mot);
        send(socket_client_2, buffer, strlen(buffer) + 1, 0);
        printf("Envoyé au client 2 : %s\n", buffer);

        // Boucle de transit des messages
        printf("\n=== Début de la partie ===\n");
        int partie_en_cours = 1;
        while (partie_en_cours) {
            // Recevoir lettre du client 2
            memset(messageRecu, 0, LG_MESSAGE);
            lus = recv(socket_client_2, messageRecu, LG_MESSAGE-1, 0);
            if (lus <= 0) {
                printf("Le client 2 s'est déconnecté.\n");
                break;
            }
            messageRecu[lus] = '\0';
            printf("Reçu du client 2 (lettre) : %s → Transite au client 1\n", messageRecu);

            // Transiter au client 1
            send(socket_client_1, messageRecu, strlen(messageRecu) + 1, 0);

            // Recevoir réponse du client 1
            memset(messageRecu, 0, LG_MESSAGE);
            lus = recv(socket_client_1, messageRecu, LG_MESSAGE-1, 0);
            if (lus <= 0) {
                printf("Le client 1 s'est déconnecté.\n");
                break;
            }
            messageRecu[lus] = '\0';
            printf("Reçu du client 1 (réponse) : %s → Transite au client 2\n", messageRecu);

            // Transiter au client 2
            send(socket_client_2, messageRecu, strlen(messageRecu) + 1, 0);

            // Vérifier fin de partie
            if (strstr(messageRecu, "gagne") != NULL || strstr(messageRecu, "perdu") != NULL) {
                printf(">>> Partie terminée !\n");
                partie_en_cours = 0;
            }
        }

        // Fermeture
        close(socket_client_1);
        close(socket_client_2);
        
        printf("\n=== Fin de la partie ===\n");
        printf("Le mot à deviner était : %s\n", mot_secret);
        printf("Connexions fermées. Prêt pour une nouvelle partie.\n\n");
    }

    close(socketEcoute);
    return 0;
}
