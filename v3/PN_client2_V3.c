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

void joueur_choisi_lettre(int socket_client) {
    char lettre[2];
    printf("\nEntrez une lettre : ");
    if (scanf(" %c", &lettre[0]) != 1) {
        printf("Erreur de saisie, réessayez.\n");
        joueur_choisi_lettre(socket_client);
    }

    int c;
    int chars_ignores = 0;
    while ((c = getchar()) != '\n' && c != EOF) {
        chars_ignores++;
    }
    if (chars_ignores > 0) {
        printf("⚠ Attention : seule la première lettre a été prise en compte.\n");
    }
    
    lettre[0] = toupper(lettre[0]);

    if (!isalpha(lettre[0])) {
        printf("Veuillez entrer une lettre valide (A-Z)\n");
        joueur_choisi_lettre(socket_client);
        return;
    } 

    emit(JOUEUR_2_PROPOSE_LETTRE, lettre, socket_client);     
}


/**
 * Client 2 - Celui qui devine le mot
 * Propose des lettres et reçoit les réponses
 */
int main(int argc, char *argv[]){
    int socket_client;
    struct sockaddr_in adresseServeur;
    socklen_t longueurAdresse;
    char buffer[LG_MESSAGE];
    int octets_lus;
    char ip_serveur[16];
    int port_serveur;
    int partie_en_cours = 1;

    int reponse_code; // ex : 2003
    char reponse_message[LG_MESSAGE];
    
    // Variables pour parser les réponses
    char status[20];
    char mot_decouvert[LG_MESSAGE];

    int longueur_mot;
    int nb_erreurs;
    
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
    printf("Tentative de connexion à %s:%d...\n", ip_serveur, port_serveur);
    if ((connect(socket_client, (struct sockaddr *) &adresseServeur, longueurAdresse)) == -1) {
        perror("Erreur connexion");
        close(socket_client);
        exit(-3);
    }
    printf(">>> Connexion réussie!\n\n");

    while(partie_en_cours) {
        memset(buffer, 0, LG_MESSAGE);
        octets_lus = recv(socket_client, buffer, LG_MESSAGE - 1, 0);
        if (octets_lus <= 0) {
            perror("Erreur réception");
            close(socket_client);
            exit(-4);
        }

        buffer[octets_lus] = '\0';

        if (sscanf(buffer, "%4d", &reponse_code) != 1) { // récupére le code
            printf("Format de communication non respecté : %s\n", buffer);
            close(socket_client);
            exit(-5);
        }

        char *msg = strchr(buffer, ':');
        if (msg != NULL) { // récupere le contenu (potentiel)
            msg++; // supprime le ':'
            strcpy(reponse_message, msg);
        } else {
            printf("Format de communication non respecté/Erreur sur le contenu : %s\n", buffer);
            close(socket_client);
            exit(-6);
        }
    
        switch(reponse_code){
            case JOUEUR_2_TAILLE_MOT_ET_PEUT_JOUER:
                longueur_mot = atoi(reponse_message);
                printf("\n===========================================\n");
                printf("    BIENVENUE AU JEU DU PENDU !\n");
                printf("===========================================\n");
                printf("Mot de %d lettres à deviner\n", longueur_mot);
                printf("Vous avez droit à 6 erreurs maximum\n");
                printf("===========================================\n\n");
                
                joueur_choisi_lettre(socket_client);
          
                break;
            case JOUEUR_2_DONNEES_PARTIE:
                if (sscanf(reponse_message, "%s %s %d", status, mot_decouvert, &nb_erreurs) != 3) {
                    printf("Réponse inattendue : %s\n", buffer);
                    break;
                }

                if (strcmp(status, "oui") == 0) {
                    printf("✓ Bonne lettre !\n");
                } else if (strcmp(status, "non") == 0) {
                    printf("✗ Mauvaise lettre.\n");
                } else if (strcmp(status, "deja") == 0) {
                    printf("⚠ Lettre déjà choisie. Choisissez-en une autre.\n");
                } else if (strcmp(status, "erreur") == 0) {
                    printf("⚠ Caractère invalide. Entrez une lettre (A-Z).\n");
                }

                afficher_pendu(nb_erreurs);

                printf("Mot à deviner : %s\n", mot_decouvert);
                
                // Fin de partie
                if (strcmp(status, "gagne") == 0) {
                    printf("\n");
                    printf("*******************************************\n");
                    printf("  FÉLICITATIONS ! VOUS AVEZ GAGNÉ !\n");
                    printf("  Le mot était : %s\n", mot_decouvert);
                    printf("  Nombre d'erreurs : %d\n", nb_erreurs);
                    printf("*******************************************\n");
                    partie_en_cours = 0;
                    break;
                } else if (strcmp(status, "perdu") == 0) {
                    printf("\n");
                    printf("*******************************************\n");
                    printf("  DOMMAGE ! VOUS AVEZ PERDU !\n");
                    printf("  Le mot était : %s\n", mot_decouvert);
                    printf("*******************************************\n");
                    partie_en_cours = 0;
                    break;
                } else {
                    joueur_choisi_lettre(socket_client);
                }
                break;
            case MESSAGE:
                printf("%s\n", reponse_message);
                break;    
            default:
                printf("Code inconnu reçu : %d (contenu: %s)\n", reponse_code, reponse_message);
                break;
        }
    }
    
    close(socket_client);
    printf("\nConnexion fermée. Au revoir !\n");
    return 0;
}
