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
    
    // Variables pour parser les réponses
    char status[20];
    char mot_decouvert[LG_MESSAGE];
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
    
    close(socket_client);
    printf("\nConnexion fermée. Au revoir !\n");
    return 0;
}
