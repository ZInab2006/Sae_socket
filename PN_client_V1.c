#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define LG_MESSAGE 512
#define CLEAR_SCREEN "\033[2J\033[H"
#define ERREURS_MAX 6

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


void clear_screen() {
    printf(CLEAR_SCREEN); // Efface l'écran et replace le curseur en haut à gauche
    fflush(stdout);          // force l'affichage
}

void display_menu_pendu(){
    printf("\n===========================================\n");
    printf("    BIENVENUE AU JEU DU PENDU !\n");
    printf("===========================================\n");
}

void display_menu_encours(int len_mot, int nb_erreurs){
    printf("Mot de %d lettres à deviner\n", len_mot);
    printf("Vous avez droit à 6 erreurs maximum\n");
    printf("===========================================\n\n");
    printf("Erreurs : %d/%d\n\n", nb_erreurs, ERREURS_MAX);
}


void display_global_menu(int len_mot, int nb_erreurs){
    display_menu_pendu();
    display_menu_encours(len_mot, nb_erreurs);
}


int main(int argc, char *argv[]){
    //=======================================================
    //              PARTIE DECLARATIVE
    //=======================================================
    int descripteurSocket;
    struct sockaddr_in adresseServeur;
    socklen_t longueurAdresse;
    
    char ip_dest[16];
    int port_dest;
    
    char buffer[LG_MESSAGE];
    int nb;
    
    //=======================================================
    //      PARTIE GESTION SOCKET DIALOGUE CLIENT
    //=======================================================
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

    
    //=======================================================
    //          PARTIE BOUCLE JEU
    //=======================================================
    int len_mot;
    int nb_erreurs = 0;
    int valueParam;
    int fin_partie = 0;

    while(!fin_partie){
        // Reception des messages du serveur
        //==================================
        memset(buffer, 0, LG_MESSAGE);
        nb = recv(descripteurSocket, buffer, LG_MESSAGE, 0);
        if (nb <= 0) {
            perror("Erreur réception");
            close(descripteurSocket);
            exit(-4);
        }
        buffer[nb] = '\0';

        // Recuperation de l'entete du message
        char *separateur = strchr(buffer, '|');
        
        if (separateur == NULL){
            perror("Format incorrect envoyé par le serveur");
            close(descripteurSocket);
            exit(-5);
        }
        
        // Premier parsing pour identifier le comportement cient
        *separateur = '\0';
        char *entete = buffer;
        char *message = separateur + 1;
        
        // Deuxieme parsing pour identifier des données optionnelles
        char *separateurValue = strchr(message, '$');
        
        if (separateurValue != NULL){
            *separateurValue = '\0';
            char *value_str = message;
            message = separateurValue + 1;
            valueParam = atoi(value_str);
        }


        // Actions clients
        //==================================

        // Initialisation informations
        if (strcmp(entete, "INIT") == 0){
            len_mot = valueParam;
            clear_screen();
            display_global_menu(len_mot, nb_erreurs);
            printf("%s",message);
            fflush(stdout);
        }
        // En attente du tour de l'autre joueur
        else if (strcmp(entete, "WAIT") == 0){
            clear_screen();
            nb_erreurs = valueParam;
            display_global_menu(len_mot, nb_erreurs);
            printf("%s", message);
            afficher_pendu(nb_erreurs);
            fflush(stdout);
        }
        else if (strcmp(entete, "FIN") == 0){
            clear_screen();
            nb_erreurs = valueParam;
            display_global_menu(len_mot, nb_erreurs);
            afficher_pendu(nb_erreurs);
            printf("\n*******************************************\n");
            printf("%s", message);
            printf("\n*******************************************\n");
            fflush(stdout);
            fin_partie = 1;
        }
        // Tour de jeu
        else if (strcmp(entete, "PLAY") == 0){
            clear_screen();
            nb_erreurs = valueParam;
            display_global_menu(len_mot, nb_erreurs);
            printf("%s", message);
            afficher_pendu(nb_erreurs);
            fflush(stdout);

            int tour_termine = 0;
            // boucle tant qu'un coup valide n'est pas joué
            while(!tour_termine){
                char lettre;
                int saisie_valide = 0;
    
                while(!saisie_valide){
                    
                    // Saisir lettre
                    printf("\nEntrez une lettre : ");
                    if (scanf(" %c", &lettre) != 1) {
                        printf("Erreur de saisie\n");
                        int c;
                        while ((c = getchar()) != '\n' && c != EOF);
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
    
                    saisie_valide = 1;
                }
                
    
                // Envoyer lettre (L175 Serv)
                snprintf(buffer, LG_MESSAGE, "%c", lettre);
                nb = send(descripteurSocket, buffer, strlen(buffer) + 1, 0);
                if (nb <= 0) {
                    perror("Erreur envoi lettre");
                    close(descripteurSocket);
                    exit(-6);
                }
                
                // Recevoir réponse (L189 serv)
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
                
                // maj affichage
                nb_erreurs = erreurs;
                clear_screen();
                display_global_menu(len_mot, nb_erreurs);
                afficher_pendu(nb_erreurs);

                // Afficher le mot avec espaces pour meilleure lisibilité
                printf("\n");
                printf("Mot : ");
                for (int i = 0; i < strlen(mot_actuel); i++) {
                    printf("%c ", mot_actuel[i]);
                }
                printf("\n");
                printf("Erreurs : %d/6\n", erreurs);
    
                // Gérer les différents cas de jeu
                if (strcmp(status, "gagne") == 0) {
                    printf("\n");
                    printf("*******************************************\n");
                    printf("  FÉLICITATIONS ! VOUS AVEZ GAGNÉ !\n");
                    printf("  Le mot était : %s\n", mot_actuel);
                    printf("  Nombre d'erreurs : %d\n", erreurs);
                    printf("*******************************************\n");
                    fin_partie = 1;
                    tour_termine = 1;
                } else if (strcmp(status, "perdu") == 0) {
                    printf("\n");
                    printf("*******************************************\n");
                    printf("  DOMMAGE ! VOUS AVEZ PERDU !\n");
                    printf("  Le mot était : %s\n", mot_actuel);
                    printf("*******************************************\n");
                    fin_partie = 1;
                    tour_termine = 1;
                } else if (strcmp(status, "elimine") == 0){
                    printf("\n");
                    printf("*******************************************\n");
                    printf("  VOUS ETES ELIMINE!\n");
                    printf("  Le mot était : %s\n", mot_actuel);
                    printf("*******************************************\n");
                    fin_partie = 1;
                    tour_termine = 1;
                } else if (strcmp(status, "deja") == 0) {
                    // lettre deja utilisee retour au debut de la boucle
                    printf("⚠ Lettre déjà choisie. Choisissez-en une autre.\n");
                } else if (strcmp(status, "oui") == 0) {
                    // tour terminé
                    printf("✓ Bonne lettre !\n");
                    tour_termine = 1;
                } else if (strcmp(status, "non") == 0) {
                    // tour terminé
                    printf("✗ Mauvaise lettre.\n");
                    tour_termine = 1;
                } else if (strcmp(status, "erreur") == 0) {
                    // lettre invalide retour au debut de la boucle
                    printf("⚠ Caractère invalide. Entrez une lettre (A-Z).\n");
                }

                sleep(1);

            }

        }

    }

    // Fermer
    close(descripteurSocket);
    printf("\nConnexion fermée. Au revoir !\n");
    return 0;
}
