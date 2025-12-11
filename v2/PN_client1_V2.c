#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define LG_MESSAGE 256
#define MAX_MOT 50
#define MAX_ERREURS 6

/**
 * Client 1 - Celui qui fait deviner le mot
 * Entre le mot secret et vérifie les lettres proposées
 */
int main(){
    int socket_client;
    struct sockaddr_in adresseServeur;
    char messageRecu[LG_MESSAGE];
    char mot_secret[MAX_MOT];  /** Mot secret à faire deviner */
    char mot_affiche[MAX_MOT];  /** Mot avec lettres découvertes (ex: "P__DU") */
    char lettres_deja_testees[26] = {0};  /** Tableau pour éviter les lettres déjà testées */
    int lus;
    int nb_erreurs = 0;

    // Création socket
    socket_client = socket(AF_INET, SOCK_STREAM, 0);

    // Configuration adresse serveur
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_port = htons(5000);
    adresseServeur.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connexion
    connect(socket_client, (struct sockaddr*)&adresseServeur, sizeof(adresseServeur));

    // Reçoit demande serveur
    lus = recv(socket_client, messageRecu, LG_MESSAGE-1, 0);
    messageRecu[lus] = '\0';
    printf("%s", messageRecu);

    // Saisie du mot
    scanf("%s", mot_secret);
    
    // Conversion en majuscules
    for(int i = 0; mot_secret[i]; i++) {
        mot_secret[i] = toupper(mot_secret[i]);
    }

    // Initialisation avec des tirets
    int longueur_mot = strlen(mot_secret);
    for(int i=0;i<longueur_mot;i++)
        mot_affiche[i] = '_';
    mot_affiche[longueur_mot] = '\0';

    // Envoi du mot
    send(socket_client, mot_secret, strlen(mot_secret)+1, 0);

    // Boucle principale
    while(1){
        lus = recv(socket_client, messageRecu, LG_MESSAGE-1, 0);
        if(lus <= 0) break;

        messageRecu[lus] = '\0';
        char lettre = toupper(messageRecu[0]);
        
        // Vérification lettre valide
        if(!isalpha(lettre)) {
            snprintf(messageRecu, LG_MESSAGE, "erreur %s %d", mot_affiche, nb_erreurs);
            send(socket_client, messageRecu, strlen(messageRecu)+1, 0);
            continue;
        }

        // Vérification si déjà testée
        if(lettres_deja_testees[lettre - 'A']){
            snprintf(messageRecu, LG_MESSAGE, "deja %s %d", mot_affiche, nb_erreurs);
            send(socket_client, messageRecu, strlen(messageRecu)+1, 0);
            continue;
        }
        lettres_deja_testees[lettre - 'A'] = 1;

        // Recherche de la lettre dans le mot
        int trouve = 0;
        for(int i=0;i<longueur_mot;i++){
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
        } else if(nb_erreurs >= MAX_ERREURS) {
            strcpy(status, "perdu");
            partie_terminee = 1;
        } else {
            strcpy(status, trouve ? "oui" : "non");
        }

        // Envoi de la réponse
        if(partie_terminee) {
            snprintf(messageRecu, LG_MESSAGE, "%s %s %d", status, 
                     partie_terminee && strcmp(status, "perdu") == 0 ? mot_secret : mot_affiche, 
                     nb_erreurs);
        } else {
            snprintf(messageRecu, LG_MESSAGE, "%s %s %d", status, mot_affiche, nb_erreurs);
        }

        send(socket_client, messageRecu, strlen(messageRecu)+1, 0);
        
        if(partie_terminee) {
            break;
        }
    }

    close(socket_client);
    return 0;
}
