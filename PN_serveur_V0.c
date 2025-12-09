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

int main(int argc, char *argv[]){
    int socketEcoute;
    struct sockaddr_in pointDeRencontreLocal;
    socklen_t longueurAdresse;
    int socketDialogue;
    struct sockaddr_in pointDeRencontreDistant;
    char messageRecu[LG_MESSAGE];
    int lus;

    // Création socket
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if (socketEcoute < 0) {
        perror("socket");
        exit(-1);
    }
    printf("Socket créée avec succès ! (%d)\n", socketEcoute);

    // Option pour réutiliser l'adresse immédiatement
    int opt = 1;
    setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Remplissage pointDeRencontreLocal
    longueurAdresse = sizeof(pointDeRencontreLocal);
    memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
    pointDeRencontreLocal.sin_family = PF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    pointDeRencontreLocal.sin_port = htons(PORT);

    // Bind
    if ((bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse)) < 0) {
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

    // Boucle serveur
    while (1) {
        printf("\n=== Attente d'une demande de connexion (quitter avec Ctrl-C) ===\n");

        // Accept
        socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &longueurAdresse);
        if (socketDialogue < 0) {
            perror("accept");
            continue;
        }
        printf(">>> Connexion acceptée depuis %s:%d\n", 
               inet_ntoa(pointDeRencontreDistant.sin_addr),
               ntohs(pointDeRencontreDistant.sin_port));

        // Initialiser partie
        int len_mot = strlen(MOT_SECRET);
        char mot_actuel[MAX_MOT];
        memset(mot_actuel, '-', len_mot);
        mot_actuel[len_mot] = '\0';
        int erreurs = 0;
        int lettres_choisies[26] = {0};  // Pour tracker les lettres (A-Z)

        // Envoyer "start x"
        char buffer[LG_MESSAGE];
        snprintf(buffer, LG_MESSAGE, "start %d", len_mot);
        send(socketDialogue, buffer, strlen(buffer) + 1, 0);
        printf("Envoyé : %s\n", buffer);
        printf("Mot à deviner : %s\n", MOT_SECRET);

        // Boucle partie
        while (1) {
            // Recevoir lettre
            memset(messageRecu, 0, LG_MESSAGE);
            lus = recv(socketDialogue, messageRecu, LG_MESSAGE, 0);
            if (lus <= 0) {
                printf("Connexion fermée par le client.\n");
                break;
            }
            messageRecu[lus] = '\0';
            
            // Récupérer la première lettre et la convertir en majuscule
            char lettre = toupper(messageRecu[0]);
            printf("Lettre reçue : %c\n", lettre);

            // Vérifier si c'est bien une lettre
            if (!isalpha(lettre)) {
                snprintf(buffer, LG_MESSAGE, "erreur %s %d", mot_actuel, erreurs);
                send(socketDialogue, buffer, strlen(buffer) + 1, 0);
                printf("Envoyé : %s (caractère invalide)\n", buffer);
                continue;
            }

            // Vérifier si déjà choisie
            int index = lettre - 'A';
            if (lettres_choisies[index]) {
                snprintf(buffer, LG_MESSAGE, "deja %s %d", mot_actuel, erreurs);
                send(socketDialogue, buffer, strlen(buffer) + 1, 0);
                printf("Envoyé : %s (lettre déjà choisie)\n", buffer);
                continue;
            }

            // Marquer la lettre comme choisie
            lettres_choisies[index] = 1;

            // Chercher la lettre dans le mot
            int trouve = 0;
            for (int i = 0; i < len_mot; i++) {
                if (MOT_SECRET[i] == lettre) {
                    mot_actuel[i] = MOT_SECRET[i];
                    trouve = 1;
                }
            }

            // Préparer réponse
            if (trouve) {
                snprintf(buffer, LG_MESSAGE, "oui %s %d", mot_actuel, erreurs);
                printf("Bonne lettre ! Mot actuel : %s\n", mot_actuel);
            } else {
                erreurs++;
                snprintf(buffer, LG_MESSAGE, "non %s %d", mot_actuel, erreurs);
                printf("Mauvaise lettre ! Erreurs : %d/%d\n", erreurs, MAX_ERREURS);
            }

            // Vérifier fin de partie
            if (strcmp(mot_actuel, MOT_SECRET) == 0) {
                snprintf(buffer, LG_MESSAGE, "gagne %s %d", mot_actuel, erreurs);
                send(socketDialogue, buffer, strlen(buffer) + 1, 0);
                printf(">>> PARTIE GAGNÉE ! Envoyé : %s\n", buffer);
                break;
            } else if (erreurs >= MAX_ERREURS) {
                snprintf(buffer, LG_MESSAGE, "perdu %s %d", MOT_SECRET, erreurs);
                send(socketDialogue, buffer, strlen(buffer) + 1, 0);
                printf(">>> PARTIE PERDUE ! Envoyé : %s\n", buffer);
                break;
            } else {
                send(socketDialogue, buffer, strlen(buffer) + 1, 0);
                printf("Envoyé : %s\n", buffer);
            }
        }

        // Fermer connexion pour cette partie
        close(socketDialogue);
        printf("\n>>> Connexion fermée. Prêt pour une nouvelle partie.\n");
    }

    // Fermer socket écoute (jamais atteint)
    close(socketEcoute);
    return 0;
}