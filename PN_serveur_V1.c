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
#define CLEAR_SCREEN "\033[2J\033[H"

void clear_screen() {
    printf(CLEAR_SCREEN); // Efface l'écran et replace le curseur en haut à gauche
    fflush(stdout);          // force l'affichage
}

int main(int argc, char *argv[]){
    //=======================================================
    //              PARTIE DECLARATIVE
    //=======================================================
    int socketEcoute;
    struct sockaddr_in adresseServeur;
    socklen_t longueurAdresse;

    int socketDialogueClient1;
    struct sockaddr_in adresseClient1;

    int socketDialogueClient2;
    struct sockaddr_in adresseClient2;

    char messageRecu[LG_MESSAGE];
    int lus;

    //=======================================================
    //      PARTIE GESTION SOCKET ECOUTE
    //=======================================================

    // Création socket d'écoute
    clear_screen();
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if (socketEcoute < 0) {
        perror("socket");
        exit(-1);
    }
    printf("Socket créée avec succès ! (%d)\n", socketEcoute);

    // Option pour réutiliser l'adresse immédiatement
    int opt = 1;
    setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Remplissage adresseServeur
    longueurAdresse = sizeof(adresseServeur);
    memset(&adresseServeur, 0x00, longueurAdresse);
    adresseServeur.sin_family = PF_INET;
    adresseServeur.sin_addr.s_addr = htonl(INADDR_ANY);
    adresseServeur.sin_port = htons(PORT);

    // Bind socket - Adresse
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



    //=======================================================
    //          PARTIE BOUCLE SERVEUR
    //=======================================================
    while (1) {

        //          INITIALISATION PARTIE
        //===================================================
        int len_mot = strlen(MOT_SECRET);
        char mot_actuel[MAX_MOT];

        memset(mot_actuel, '-', len_mot);
        mot_actuel[len_mot] = '\0';

        int erreurs_J1 = 0;
        int erreurs_J2 = 0;
        int j1_elimine = 0; // passe à 1 si max erreur
        int j2_elimine = 0; // passe à 1 si max erreur
        int lettres_choisies[26] = {0};  // Pour tracker les lettres (A-Z)



        //      INITIALISATION SOCKET CONNEXION CLIENT
        //===================================================
        char buffer[LG_MESSAGE];

        printf("\n=== Attente d'une demande de connexion (quitter avec Ctrl-C) ===\n");

        // Acceptation Client 1
        socketDialogueClient1 = accept(socketEcoute, (struct sockaddr *) &adresseClient1, &longueurAdresse);
        if (socketDialogueClient1 < 0) {
            perror("accept client1");
            continue;
        }
        printf("\n>>> Connexion Client1 acceptée depuis %s:%d\n", 
               inet_ntoa(adresseClient1.sin_addr),
               ntohs(adresseClient1.sin_port));
        
        // Faire patienter le client 1 pour trouver le mot de x lettres"
        snprintf(buffer, LG_MESSAGE, "INIT|%d$Vous êtes connecté.\nVeuillez attendre la connexion d'un deuxième joueur pour trouver un mot de %d lettres.", len_mot, len_mot);
        send(socketDialogueClient1, buffer, strlen(buffer) + 1, 0);
        printf("Message d'attente envoyé au client 1\n");
  
        // Acceptation Client 2
        printf("En attente d'une deuxieme connexion...\n");
        socketDialogueClient2 = accept(socketEcoute, (struct sockaddr *) &adresseClient2, &longueurAdresse);
        if (socketDialogueClient2 < 0) {
            perror("accept");
            continue;
        }
        printf("\n>>> Connexion Client2 acceptée depuis %s:%d\n", 
               inet_ntoa(adresseClient2.sin_addr),
               ntohs(adresseClient2.sin_port));
            
        // Lancer la partie sur le client 2
        memset(buffer, 0, LG_MESSAGE);
        snprintf(buffer, LG_MESSAGE, "INIT|%d$La partie commence !\n", len_mot);
        send(socketDialogueClient2, buffer, strlen(buffer) + 1, 0);
        printf("\n>>> Envoi du INIT au joueur2.\n");
        sleep(1);



        //          BOUCLE PARTIE PENDU
        //===================================================
        printf("\nMot à deviner : %s\n", MOT_SECRET);
        
        int joueur_actif = 2;
        int socket_active;
        int socket_inactive;
        int erreurs_joueur_actif;
        int erreurs_joueur_inactif;
        int fin_partie = 0;

        while (!fin_partie) {
            int joueur_inactif;
            int inactif_elimine;
            
            // bascule sur l'autre joueur si l'un est éliminé
            if (joueur_actif == 1 && j1_elimine) {
                joueur_actif = 2;
            } else if (joueur_actif == 2 && j2_elimine) {
                joueur_actif = 1;
            }

            // Initialisation des données des joueurs
            if (joueur_actif == 1){
                socket_active = socketDialogueClient1;
                socket_inactive = socketDialogueClient2;
                erreurs_joueur_actif = erreurs_J1;
                erreurs_joueur_inactif = erreurs_J2;
                joueur_inactif = 2;
                inactif_elimine = j2_elimine;
                printf("Joueur Actif : Joueur 1\n");
            }
            else{
                socket_active = socketDialogueClient2;
                socket_inactive = socketDialogueClient1;
                erreurs_joueur_actif = erreurs_J2;
                erreurs_joueur_inactif = erreurs_J1;
                joueur_inactif = 1;
                inactif_elimine = j1_elimine;
                printf("Joueur Actif : Joueur 2\n");
            }

            // Un joueur joue, un joeuur en attente
            memset(buffer, 0, LG_MESSAGE);
            snprintf(buffer, LG_MESSAGE, "PLAY|%d$\nA vous de jouer !\n\n\t%s", erreurs_joueur_actif, mot_actuel);
            send(socket_active, buffer, strlen(buffer) + 1, 0);
            
            if(!inactif_elimine){
                memset(buffer, 0, LG_MESSAGE);
                snprintf(buffer, LG_MESSAGE, "WAIT|%d$\nEn attente, joueur %d joue !\n\n\t%s\n", erreurs_joueur_inactif, joueur_actif, mot_actuel);
                send(socket_inactive, buffer, strlen(buffer) + 1, 0);
            }


            // Recevoir lettre (L262 client)
            memset(messageRecu, 0, LG_MESSAGE);
            lus = recv(socket_active, messageRecu, LG_MESSAGE, 0);
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
                memset(buffer, 0, LG_MESSAGE);
                snprintf(buffer, LG_MESSAGE, "erreur %s %d", mot_actuel, erreurs_joueur_actif);
                send(socket_active, buffer, strlen(buffer) + 1, 0);
                printf("Envoyé : %s (caractère invalide)\n", buffer);
                continue;
            }

            // Vérifier si déjà choisie
            int index = lettre - 'A';

            if (lettres_choisies[index]) {
                memset(buffer, 0, LG_MESSAGE);
                snprintf(buffer, LG_MESSAGE, "deja %s %d", mot_actuel, erreurs_joueur_actif);
                send(socket_active, buffer, strlen(buffer) + 1, 0);
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
                memset(buffer, 0, LG_MESSAGE);
                snprintf(buffer, LG_MESSAGE, "oui %s %d", mot_actuel, erreurs_joueur_actif);
                printf("Bonne lettre ! Mot actuel : %s\n", mot_actuel);
            } else {
                erreurs_joueur_actif++;
                memset(buffer, 0, LG_MESSAGE);
                snprintf(buffer, LG_MESSAGE, "non %s %d", mot_actuel, erreurs_joueur_actif);
                printf("Mauvaise lettre ! Erreurs : %d/%d\n", erreurs_joueur_actif, MAX_ERREURS);
            }

            // maj des erreurs joueurs
            if (joueur_actif == 1){
                erreurs_J1 = erreurs_joueur_actif;
            }
            else{
                erreurs_J2 = erreurs_joueur_actif;
            }

            // ==========================================
            //         GESTION FINS DE PARTIE
            // ==========================================
            
            // Le joueur actif trouve le mot = GAGNE 
            if (strcmp(mot_actuel, MOT_SECRET) == 0) {
                memset(buffer, 0, LG_MESSAGE);
                snprintf(buffer, LG_MESSAGE, "gagne %s %d", mot_actuel, erreurs_joueur_actif);
                send(socket_active, buffer, strlen(buffer) + 1, 0);
                printf(">>> Joueur %d Gagne la partie ! Envoyé : %s\n",joueur_actif, buffer);

                memset(buffer, 0, LG_MESSAGE);
                snprintf(buffer, LG_MESSAGE, "FIN|%d$\nPARTIE TERMINÉE\n\nLe Joueur %d a trouvé le mot : %s\n\nVous avez perdu !", erreurs_joueur_inactif, joueur_actif, MOT_SECRET);
                send(socket_inactive, buffer, strlen(buffer) + 1, 0);
                
                fin_partie = 1;
            } 
            // Le joueur actif atteint le quota d'erreurs
            else if (erreurs_joueur_actif >= MAX_ERREURS) {
                // Eliminer le joueur actif
                if (joueur_actif == 1){
                    j1_elimine = 1;
                }
                else{
                    j2_elimine = 1;
                }

                // verif par rapport a l'autre joueur
                if (j1_elimine && j2_elimine){
                    // Cas où les deux joueurs sont eliminés
                    memset(buffer, 0, LG_MESSAGE);
                    snprintf(buffer, LG_MESSAGE, "perdu %s %d Les deux joueurs ont perdu. Vous n'avez pas trouvé le mot secret!", MOT_SECRET, erreurs_joueur_actif);
                    send(socket_active, buffer, strlen(buffer) + 1, 0);
                    
                    memset(buffer, 0, LG_MESSAGE);
                    snprintf(buffer, LG_MESSAGE, "perdu %s %d Les deux joueurs ont perdu. Vous n'avez pas trouvé le mot secret!", MOT_SECRET, erreurs_joueur_inactif);
                    send(socket_inactive, buffer, strlen(buffer) + 1, 0);

                    printf("\nLes deux joueurs ont perdu.\n");

                    fin_partie = 1;
                }
                else{
                    // le joueur inactif peut continuer
                    memset(buffer, 0, LG_MESSAGE);
                    snprintf(buffer, LG_MESSAGE, "elimine %s %d Vous avez atteint le max d'erreurs, vous êtes éliminé", MOT_SECRET, erreurs_joueur_actif);
                    send(socket_active, buffer, strlen(buffer) + 1, 0);
                    printf("joueur %d éliminé.", joueur_actif);

                    // Changer de joueur et continuer
                    joueur_actif = (joueur_actif == 1) ? 2 : 1;
                    continue; 
                }
            }
            // partie continue normalement
            else{
                send(socket_active, buffer, strlen(buffer) + 1, 0);
                printf("Envoyé : %s\n", buffer);

                // Changement de joueur actif en fonction des elimines
                if (!inactif_elimine) {
                    joueur_actif = (joueur_actif == 1) ? 2 : 1;
                }
                else{
                    sleep(1);
                }
            }

        }

        // Fermer connexion pour cette partie
        close(socketDialogueClient1);
        close(socketDialogueClient2);
        printf("\n>>> Connexion fermée. Prêt pour une nouvelle partie.\n");
    }

    // Fermer socket écoute (jamais atteint)
    close(socketEcoute);
    return 0;
}
