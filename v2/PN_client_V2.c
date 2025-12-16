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

/**
 * Client V2 - Peut être Client 1 (fait deviner) ou Client 2 (devine)
 * 
 * Usage:
 *   ./PN_client_V2 <role> <ip> <port>
 *   role: 1 pour faire deviner, 2 pour deviner
 *   Exemple: ./PN_client_V2 1 127.0.0.1 5000  (Client 1)
 *            ./PN_client_V2 2 127.0.0.1 5000  (Client 2)
 */
int main(int argc, char *argv[]){
    int socket_client;
    struct sockaddr_in adresseServeur; // Va stocker l'IP, le port et le type d'adresse du serveur
    socklen_t longueurAdresse;
    char ip_serveur[16];
    int port_serveur;
    int role; // 1 = fait deviner, 2 = devine
    
    // Récupération arguments
    if (argc < 4) {
        printf("USAGE : %s <role> <ip> <port>\n", argv[0]);
        printf("  role: 1 pour faire deviner, 2 pour deviner\n");
        printf("Exemple : %s 1 127.0.0.1 5000\n", argv[0]);
        printf("          %s 2 127.0.0.1 5000\n", argv[0]);
        exit(-1);
    }
    
    // Récupération rôle
    sscanf(argv[1], "%d", &role);
    if (role != 1 && role != 2) {
        printf("Erreur : Veuillez entrer 1 pour faire deviner ou 2 pour deviner\n");
        exit(-1);
    }
    
    // Récupération IP et port
    strncpy(ip_serveur, argv[2], 15);
    ip_serveur[15] = '\0';
    sscanf(argv[3], "%d", &port_serveur);
    
    // Création socket AF_INET pour IPv4, SOCK_STREAM pour TCP, 0 pour le protocole par défaut
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
    adresseServeur.sin_port = htons(port_serveur); // htons convertit le port format machine en format réseau
    if (inet_aton(ip_serveur, &adresseServeur.sin_addr) == 0) { // inet_aton convertit l'IP en format binaire
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
    
    // ============================================================
    // CLIENT 1 : Fait deviner le mot
    // ============================================================
    if (role == 1) {
        char messageRecu[LG_MESSAGE];
        char mot_secret[MAX_MOT];
        char mot_affiche[MAX_MOT];
        char lettres_deja_testees[26] = {0};
        int lus;
        int nb_erreurs = 0;
        
        // Reçoit demande serveur, qui demande le mot à deviner
        lus = recv(socket_client, messageRecu, LG_MESSAGE-1, 0);
        if (lus <= 0) {
            perror("Erreur réception");
            close(socket_client);
            exit(-4);
        }
        // \0 permet de terminer la chaîne de caractères pour éviter les problèmes de dépassement de buffer
        messageRecu[lus] = '\0';
        printf("%s", messageRecu);
        
        // Saisie du mot
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
        
        // Envoi du mot
        send(socket_client, mot_secret, strlen(mot_secret)+1, 0);
        
        // Boucle principale
        while(1){
            lus = recv(socket_client, messageRecu, LG_MESSAGE-1, 0);
            if(lus <= 0) break;
            
            messageRecu[lus] = '\0';
            char lettre = toupper((unsigned char)messageRecu[0]);
            
            // Vérification lettre valide
            if(!isalpha((unsigned char)lettre)) {
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
    }
    
    // ============================================================
    // CLIENT 2 : Devine le mot
    // ============================================================
    else {  // role == 2
        char buffer[LG_MESSAGE];
        int octets_lus;
        char status[20];
        char mot_decouvert[LG_MESSAGE];
        int nb_erreurs;
        
        // Réception "start x"
        memset(buffer, 0, LG_MESSAGE);
        octets_lus = recv(socket_client, buffer, LG_MESSAGE, 0);
        if (octets_lus <= 0) {
            perror("Erreur réception");
            close(socket_client);
            exit(-4);
        }
        buffer[octets_lus] = '\0';
        
        int longueur_mot;
        // On attend le message "start x" où x est la longueur du mot à deviner
        if (sscanf(buffer, "start %d", &longueur_mot) != 1) {
            printf("Message inattendu : %s\n", buffer);
            close(socket_client);
            exit(-5);
        }
        
        printf("\n===========================================\n");
        printf("    BIENVENUE AU JEU DU PENDU !\n");
        printf("===========================================\n");
        printf("Mot de %d lettres à deviner\n", longueur_mot);
        printf("Vous avez droit à 6 erreurs maximum\n");
        printf("===========================================\n\n");
        
        // Boucle principale
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
            // On nettoie le buffer d'entrée pour éviter les problèmes de dépassement de buffer
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
            sprintf(buffer, "%c", lettre);
            send(socket_client, buffer, strlen(buffer) + 1, 0);
            
            // Réception réponse
            memset(buffer, 0, LG_MESSAGE);
            octets_lus = recv(socket_client, buffer, LG_MESSAGE, 0);
            if (octets_lus <= 0) {
                perror("Erreur réception");
                close(socket_client);
                exit(-7);
            }
            buffer[octets_lus] = '\0';
            
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
            printf("Erreurs : %d/6\n", nb_erreurs);
            
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
    
    close(socket_client);
    printf("\nConnexion fermée. Au revoir !\n");
    return 0;
}

