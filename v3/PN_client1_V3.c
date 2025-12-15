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

// Voir définition des codes dans PN_serveur_V3.c

#define JOUEUR_1_ENTRE_MOT 2001 
#define JOUEUR_2_TAILLE_MOT_ET_PEUT_JOUER 2002
#define JOUEUR_2_PROPOSE_LETTRE 2003
#define JOUEUR_1_VALIDE_OU_NON 2004
#define JOUEUR_2_RECOIT_VALIDE_OU_NON 2005
#define JOUEUR_2_DONNEES_PARTIE 2006
#define MESSAGE 3001 

void emit(const int code, const char* buffer, int socket) {
    char message[LG_MESSAGE];
    sprintf(message, "%s"/*, code*/, buffer);
    send(socket, message, strlen(message), 0);
}

/**
 * Client 1 - Celui qui fait deviner le mot
 * Entre le mot secret et vérifie les lettres proposées
 */
int main(int argc, char *argv[]){
    int socket_client;
    socklen_t longueurAdresse;
    struct sockaddr_in adresseServeur;
    char ip_serveur[16];
    int port_serveur;
    char status[10];
    char messageRecu[LG_MESSAGE];
    char mot_secret[MAX_MOT];  /** Mot secret à faire deviner */
    char mot_affiche[MAX_MOT];  /** Mot avec lettres découvertes (ex: "P__DU") */
    char lettres_deja_testees[26] = {0};  /** Tableau pour éviter les lettres déjà testées */
    int lus;
    int nb_erreurs = 0;
    int partie_en_cours = 1;

    int reponse_code; // ex : 2003
    char reponse_message[LG_MESSAGE];
    int longueur_mot;

    // Récupération arguments
    if (argc < 3) {
        printf("USAGE : %s ip port\n", argv[0]);
        printf("Exemple : %s 127.0.0.1 5000\n", argv[0]);
        exit(-1);
    }
    strncpy(ip_serveur, argv[1], 15);
    ip_serveur[15] = '\0';
    sscanf(argv[2], "%d", &port_serveur);
    
    // Création socket
    socket_client = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_client < 0) {
        perror("Erreur socket");
        exit(-1);
    }
    printf("Socket créée! (%d)\n", socket_client);
    
    // Configuration adresse
    longueurAdresse = sizeof(adresseServeur);
    memset(&adresseServeur, 0x00, longueurAdresse);
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_port = htons(port_serveur);
    if (inet_aton(ip_serveur, &adresseServeur.sin_addr) == 0) {
        printf("Adresse IP invalide : %s\n", ip_serveur);
        close(socket_client);
        exit(-2);
    }

    // Connexion
    connect(socket_client, (struct sockaddr*)&adresseServeur, sizeof(adresseServeur));

    // Boucle principale
    while(partie_en_cours) {
        lus = recv(socket_client, messageRecu, LG_MESSAGE-1, 0);
        if(lus <= 0) break;

        messageRecu[lus] = '\0';

        /* Récupere les 4 premiers chiffres du message, correspondants au code à interpreter plus bas*/
        if (sscanf(messageRecu, "%4d", &reponse_code) != 1) { // récupére le code
            printf("Format de communication non respecté : %s\n", messageRecu);
            close(socket_client);
            exit(-5);
        }

        char *msg = strchr(messageRecu, ':');
        if (msg != NULL) { // récupere le contenu (potentiel)
            msg++; // supprime le ':'
            strcpy(reponse_message, msg);
        } else {
            printf("Format de communication non respecté/Erreur sur le contenu : %s\n", messageRecu);
        }

        switch(reponse_code){
            case JOUEUR_1_ENTRE_MOT:
                printf("Un joueur est arrivé !\nVeuillez entrer le mot à faire deviner : ");
                scanf("%s", mot_secret);

                for (int i = 0; mot_secret[i]; i++) 
                    mot_secret[i] = toupper((unsigned char)mot_secret[i]);

                 // Initialisation avec des tirets
                longueur_mot = strlen(mot_secret);
                for(int i=0;i<longueur_mot;i++)
                    mot_affiche[i] = '_';
                mot_affiche[longueur_mot] = '\0';
                send(socket_client, mot_secret, longueur_mot, 0);
                break;
            case JOUEUR_2_PROPOSE_LETTRE: {
                char status[20];
                int partie_terminee = 0;

                char lettre = toupper((unsigned char)reponse_message[0]);

                printf("Le joueur propose la lettre : %c\n", lettre);

                // Vérification lettre valide
                if (!isalpha((unsigned char)lettre) || lettre < 'A' || lettre > 'Z') {
                    strcpy(status, "erreur");
                    snprintf(messageRecu, LG_MESSAGE, "%s %s %d", status, mot_affiche, nb_erreurs);
                    emit(JOUEUR_2_DONNEES_PARTIE, messageRecu, socket_client);
                    break;
                }

                // Vérification si déjà testée
                if (lettres_deja_testees[lettre - 'A']) {
                    strcpy(status, "deja");
                    snprintf(messageRecu, LG_MESSAGE, "%s %s %d", status, mot_affiche, nb_erreurs);
                    emit(JOUEUR_2_DONNEES_PARTIE, messageRecu, socket_client);
                    break;
                }

                // Marquer comme testée
                lettres_deja_testees[lettre - 'A'] = 1;

                // Recherche dans le mot
                int trouve = 0;
                for (int i = 0; i < longueur_mot; i++) {
                    if (mot_secret[i] == lettre) {
                        mot_affiche[i] = lettre;
                        trouve = 1;
                    }
                }

                if (!trouve)
                    nb_erreurs++;

                // Vérifier mot complet
                int mot_complet = 1;
                for (int i = 0; i < longueur_mot; i++) {
                    if (mot_affiche[i] == '_') {
                        mot_complet = 0;
                        break;
                    }
                }
               

                if (mot_complet) {
                    strcpy(status, "gagne");
                    partie_terminee = 1;
                } else if (nb_erreurs >= MAX_ERREURS) {
                    strcpy(status, "perdu");
                    partie_terminee = 1;
                } else {
                    strcpy(status, trouve ? "oui" : "non");
                }

                if(partie_terminee && (strcmp(status, "perdu")|| strcmp(status, "gagne"))) 
                    printf("Le joueur a %s la partie ! Le mot était : %s\n", strcmp(status, "gagne") == 0 ? "gagné" : "perdu", mot_secret);
                

                // Envoi réponse
                snprintf(messageRecu, LG_MESSAGE, "%s %s %d",
                        status,
                        (partie_terminee && strcmp(status, "perdu") == 0) ? mot_secret : mot_affiche,
                        nb_erreurs);

                emit(JOUEUR_2_DONNEES_PARTIE, messageRecu, socket_client);

                if (partie_terminee) 
                    partie_en_cours = 0;


                break;
            }
            case MESSAGE:
                printf("%s\n", reponse_message);
                break;
            default:
                printf("Code reçu inconnu : %d\n", reponse_code);
                break;
        }        
    }

    close(socket_client);
    return 0;
}
