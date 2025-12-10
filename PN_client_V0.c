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

int main(int argc, char *argv[]){
    int descripteurSocket;
    struct sockaddr_in adresseServeur;
    socklen_t longueurAdresse;
    char buffer[LG_MESSAGE];
    int nb;
    char ip_dest[16];
    int port_dest;

    // Récupération IP et port
    if (argc < 3) {
        printf("USAGE : %s ip port\n", argv[0]);
        printf("Exemple : %s 127.0.0.1 5000\n", argv[0]);
        exit(-1);
    }
    strncpy(ip_dest, argv[1], 15);
    ip_dest[15] = '\0';
    sscanf(argv[2], "%d", &port_dest);

    // Création socket
    descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (descripteurSocket < 0) {
        perror("Erreur en création de la socket");
        exit(-1);
    }
    printf("Socket créée! (%d)\n", descripteurSocket);

    // Remplissage adresseServeur
    longueurAdresse = sizeof(adresseServeur);
    memset(&adresseServeur, 0x00, longueurAdresse);
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_port = htons(port_dest);
    if (inet_aton(ip_dest, &adresseServeur.sin_addr) == 0) {
        printf("Adresse IP invalide : %s\n", ip_dest);
        close(descripteurSocket);
        exit(-2);
    }

    // Connexion
    printf("Tentative de connexion à %s:%d...\n", ip_dest, port_dest);
    if ((connect(descripteurSocket, (struct sockaddr *) &adresseServeur, longueurAdresse)) == -1) {
        perror("Erreur de connexion avec le serveur distant");
        close(descripteurSocket);
        exit(-3);
    }
    printf(">>> Connexion au serveur %s:%d réussie!\n\n", ip_dest, port_dest);

    // Attendre "start x"
    memset(buffer, 0, LG_MESSAGE);
    nb = recv(descripteurSocket, buffer, LG_MESSAGE, 0);
    if (nb <= 0) {
        perror("Erreur réception start");
        close(descripteurSocket);
        exit(-4);
    }
    buffer[nb] = '\0';
    
    int len_mot;
    if (sscanf(buffer, "start %d", &len_mot) != 1) {
        printf("Message inattendu : %s\n", buffer);
        close(descripteurSocket);
        exit(-5);
    }
    
    printf("\n===========================================\n");
    printf("    BIENVENUE AU JEU DU PENDU !\n");
    printf("===========================================\n");
    printf("Mot de %d lettres à deviner\n", len_mot);
    printf("Vous avez droit à 6 erreurs maximum\n");
    printf("===========================================\n\n");

    // Boucle de jeu
    while (1) {
        // Saisir lettre
        char lettre;
        printf("\nEntrez une lettre : ");
        if (scanf(" %c", &lettre) != 1) {
            printf("Erreur de saisie\n");
            continue;
        }
        
        // Nettoyer le buffer d'entrée
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        // Convertir en majuscule pour uniformité
        lettre = toupper(lettre);

        // Vérifier si c'est une lettre
        if (!isalpha(lettre)) {
            printf("Veuillez entrer une lettre valide (A-Z)\n");
            continue;
        }

        // Envoyer lettre
        sprintf(buffer, "%c", lettre);
        nb = send(descripteurSocket, buffer, strlen(buffer) + 1, 0);
        if (nb <= 0) {
            perror("Erreur envoi lettre");
            close(descripteurSocket);
            exit(-6);
        }

        // Recevoir réponse
        memset(buffer, 0, LG_MESSAGE);
        nb = recv(descripteurSocket, buffer, LG_MESSAGE, 0);
        if (nb <= 0) {
            perror("Erreur réception réponse");
            close(descripteurSocket);
            exit(-7);
        }
        buffer[nb] = '\0';

        // Parser réponse : status mot_actuel erreurs
        char status[20], mot_actuel[LG_MESSAGE];
        int erreurs;
        if (sscanf(buffer, "%s %s %d", status, mot_actuel, &erreurs) != 3) {
            printf("Réponse inattendue : %s\n", buffer);
            continue;
        }

        // Afficher le mot avec espaces pour meilleure lisibilité
        printf("\n");
        printf("Mot : ");
        for (int i = 0; i < strlen(mot_actuel); i++) {
            printf("%c ", mot_actuel[i]);
        }
        printf("\n");
        printf("Erreurs : %d/6\n", erreurs);

        // Gérer fin
        if (strcmp(status, "gagne") == 0) {
            printf("\n");
            printf("*******************************************\n");
            printf("  FÉLICITATIONS ! VOUS AVEZ GAGNÉ !\n");
            printf("  Le mot était : %s\n", mot_actuel);
            printf("  Nombre d'erreurs : %d\n", erreurs);
            printf("*******************************************\n");
            break;
        } else if (strcmp(status, "perdu") == 0) {
            printf("\n");
            printf("*******************************************\n");
            printf("  DOMMAGE ! VOUS AVEZ PERDU !\n");
            printf("  Le mot était : %s\n", mot_actuel);
            printf("*******************************************\n");
            break;
        } else if (strcmp(status, "deja") == 0) {
            printf("⚠ Lettre déjà choisie. Choisissez-en une autre.\n");
        } else if (strcmp(status, "oui") == 0) {
            printf("✓ Bonne lettre !\n");
        } else if (strcmp(status, "non") == 0) {
            printf("✗ Mauvaise lettre.\n");
        } else if (strcmp(status, "erreur") == 0) {
            printf("⚠ Caractère invalide. Entrez une lettre (A-Z).\n");
        }
    }

    // Fermer
    close(descripteurSocket);
    printf("\nConnexion fermée. Au revoir !\n");
    return 0;
}
