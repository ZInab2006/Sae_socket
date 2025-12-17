
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
#define JOUEUR_2_PROPOSE_LETTRE_OU_MOT 2003
#define JOUEUR_1_VALIDE_OU_NON 2004
#define JOUEUR_2_RECOIT_VALIDE_OU_NON 2005
#define JOUEUR_2_DONNEES_PARTIE 2006
#define MESSAGE 3001 

void emit(const int code, const char* buffer, int socket) {
    char message[LG_MESSAGE];
    sprintf(message, "%s"/*, code*/, buffer);
    send(socket, message, strlen(message), 0);
}

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

void joueur_choisi_lettre(int socket_client, int taille_mot) {
    char buffer[1024]; // taille raisonnable côté saisie

    printf("\nEntrez une lettre ou un mot : ");

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        printf("Erreur de saisie, réessayez.\n");
        joueur_choisi_lettre(socket_client, taille_mot);
        return;
    }

    // Supprime '\n'
    buffer[strcspn(buffer, "\n")] = '\0';

    size_t len = strlen(buffer);

    if (len == 0) {
        printf("Veuillez entrer au moins un caractère.\n");
        joueur_choisi_lettre(socket_client, taille_mot);
        return;
    }

    if (len > taille_mot) {
        printf("Mot trop long.\n");
        joueur_choisi_lettre(socket_client, taille_mot);
        return;
    }

    // Vérification pour que ce soit uniquement des lettres
    for (size_t i = 0; i < len; i++) {
        if (!isalpha((unsigned char)buffer[i])) {
            printf("Uniquement des lettres (A-Z).\n");
            return;
        }
        buffer[i] = toupper(buffer[i]);
    }

    emit(JOUEUR_2_PROPOSE_LETTRE_OU_MOT, buffer, socket_client); // envoi au serveur
}

/**
 * Client - Celui qui fait deviner le mot ou devine en fonction du serveur
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
    int lus;

    // Client 1
    char mot_secret[MAX_MOT];  /** Mot secret à faire deviner */
    char mot_affiche[MAX_MOT];  /** Mot avec lettres découvertes (ex: "P__DU") */
    char lettres_deja_testees[26] = {0};  /** Tableau pour éviter les lettres déjà testées */
    int nb_erreurs = 0;
    int partie_en_cours = 1;
    int type_joueur = -1;

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
            case JOUEUR_2_PROPOSE_LETTRE_OU_MOT: {
                char status[20];
                int partie_terminee = 0;
                int trouve = 0; // si lettre trouvée = 1

                int len = strlen(reponse_message);

                if (len == 1) { // si c'est une lettre uniquement
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

                    // Marquer la lettre comme testée
                    lettres_deja_testees[lettre - 'A'] = 1;

                    // Recherche dans le mot
                    for (int i = 0; i < longueur_mot; i++) {
                        if (mot_secret[i] == lettre) {
                            mot_affiche[i] = lettre;
                            trouve = 1;
                        }
                    }

                    if (!trouve)
                        nb_erreurs++;
                }
                // Cas mot
                else {
                    printf("Le joueur propose le mot : %s\n", reponse_message);

                    // Vérifier mot complet
                    if (strcmp(reponse_message, mot_secret) == 0) {
                        strcpy(mot_affiche, mot_secret);
                    } else {
                        nb_erreurs++;
                    }
                }

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
            case JOUEUR_2_TAILLE_MOT_ET_PEUT_JOUER:
                longueur_mot = atoi(reponse_message);
                printf("\n===========================================\n");
                printf("    BIENVENUE AU JEU DU PENDU !\n");
                printf("===========================================\n");
                printf("Mot de %d lettres à deviner\n", longueur_mot);
                printf("Vous avez droit à 6 erreurs maximum\n");
                printf("===========================================\n\n");
                
                joueur_choisi_lettre(socket_client, longueur_mot);
          
                break;
            case JOUEUR_2_DONNEES_PARTIE:
                if (sscanf(reponse_message, "%s %s %d", status, mot_affiche, &nb_erreurs) != 3) {
                    printf("Réponse inattendue : %s\n",messageRecu);
                    break;
                }

                if (strcmp(status, "oui") == 0) {
                    printf("✓ Bonne lettre !\n");
                } else if (strcmp(status, "non") == 0) {
                    printf("✗ Mauvaise lettre ou mot.\n");
                } else if (strcmp(status, "deja") == 0) {
                    printf("⚠ Lettre déjà choisie. Choisissez-en une autre.\n");
                } else if (strcmp(status, "erreur") == 0) {
                    printf("⚠ Caractère invalide. Entrez une lettre (A-Z).\n");
                }

                afficher_pendu(nb_erreurs);

                printf("Mot à deviner : %s\n", mot_affiche);
                
                // Fin de partie
                if (strcmp(status, "gagne") == 0) {
                    printf("\n");
                    printf("*******************************************\n");
                    printf("  FÉLICITATIONS ! VOUS AVEZ GAGNÉ !\n");
                    printf("  Le mot était : %s\n", mot_affiche);
                    printf("  Nombre d'erreurs : %d\n", nb_erreurs);
                    printf("*******************************************\n");
                    partie_en_cours = 0;
                    break;
                } else if (strcmp(status, "perdu") == 0) {
                    printf("\n");
                    printf("*******************************************\n");
                    printf("  DOMMAGE ! VOUS AVEZ PERDU !\n");
                    printf("  Le mot était : %s\n", mot_affiche);
                    printf("*******************************************\n");
                    partie_en_cours = 0;
                    break;
                } else {
                    joueur_choisi_lettre(socket_client, longueur_mot);
                }
                break;
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

