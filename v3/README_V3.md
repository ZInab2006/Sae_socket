# JEU DU PENDU - Version 3 (V3)

## Description

Version 3 du jeu du pendu en réseau. Cette version permet à **2 joueurs** de jouer ensemble, avec la possibilité pour le serveur de gérer **plusieurs parties simultanément** grâce à l'utilisation de `fork()`.

- **Client 1** : Le joueur qui fait deviner (entre le mot secret)
- **Client 2** : Le joueur qui devine (propose des lettres)

Le **serveur** fait transiter les messages entre les 2 clients et utilise des processus séparés pour gérer chaque partie en parallèle.

## Architecture

```
Client 1 (fait deviner)  ←→  Serveur (relais + fork)  ←→  Client 2 (devine)
                                    ↓
                            [Processus enfant par partie]
```

## Améliorations par rapport à V2

- ✅ **Gestion de plusieurs parties simultanées** : Le serveur peut accepter plusieurs paires de joueurs en même temps
- ✅ **Utilisation de `fork()`** : Chaque partie est gérée dans un processus séparé
- ✅ **Système de codes de communication** : Messages formatés avec codes pour une meilleure organisation
- ✅ **Meilleure gestion des connexions** : Le serveur principal reste disponible pour de nouvelles parties

## Compilation

### Linux/macOS :
```bash
# Compiler le serveur
gcc -o PN_serveur_V3 PN_serveur_V3.c

# Compiler les clients
gcc -o PN_client1_V3 PN_client1_V3.c
gcc -o PN_client2_V3 PN_client2_V3.c
```

### Windows (avec MinGW ou WSL) :
Même commande dans WSL ou MinGW

## Exécution

### Terminal 1 - Serveur :
```bash
./PN_serveur_V3
```

### Terminal 2 - Client 1 (fait deviner) :
```bash
./PN_client1_V3
```
Le client 1 doit entrer un mot à faire deviner.

### Terminal 3 - Client 2 (devine) :
```bash
./PN_client2_V3 127.0.0.1 5000
```
Le client 2 propose des lettres pour deviner le mot.

**Note** : Vous pouvez lancer plusieurs paires de clients simultanément, le serveur gérera chaque partie dans un processus séparé.

## Règles du jeu

- Le **Client 1** entre un mot secret (maximum 50 caractères)
- Le **Client 2** doit deviner le mot en proposant des lettres
- Maximum d'erreurs : **6**
- Le pendu s'affiche progressivement avec chaque erreur
- La partie se termine quand :
  - Le mot est complètement découvert → **Victoire**
  - 6 erreurs sont commises → **Défaite**

## Format des messages

### Système de codes

Les messages sont formatés avec un code suivi de `:` et du contenu :
- Format : `CODE:contenu`

### Codes utilisés

- **2001** (`JOUEUR_1_ENTRE_MOT`) : Le serveur demande au client 1 d'entrer un mot
- **2002** (`JOUEUR_2_TAILLE_MOT_ET_PEUT_JOUER`) : Le client 2 reçoit la taille du mot et peut commencer
- **2003** (`JOUEUR_2_PROPOSE_LETTRE`) : Le client 2 propose une lettre
- **2004** (`JOUEUR_1_VALIDE_OU_NON`) : Le client 1 valide ou non la lettre
- **2005** (`JOUEUR_2_RECOIT_VALIDE_OU_NON`) : Le client 2 reçoit la validation
- **2006** (`JOUEUR_2_DONNEES_PARTIE`) : Le client 2 reçoit les données de la partie
- **3001** (`MESSAGE`) : Message informatif (ne déclenche aucune action)

### Exemples de messages

- `2001:` → Client 1 doit entrer un mot
- `2002:5` → Client 2 : mot de 5 lettres
- `2003:P` → Client 1 reçoit la lettre "P"
- `2006:oui P__DU 0` → Client 2 : bonne lettre, mot partiel, 0 erreur
- `2006:gagne PENDU 2` → Client 2 : partie gagnée
- `3001:Bienvenue joueur 1 !` → Message informatif

## Fonctionnalités

- ✅ Communication TCP/IP entre 2 clients via un serveur
- ✅ Gestion de plusieurs parties simultanées avec `fork()`
- ✅ Système de codes pour organiser les communications
- ✅ Affichage du pendu ASCII selon le nombre d'erreurs
- ✅ Gestion des lettres déjà testées
- ✅ Détection automatique de fin de partie
- ✅ Serveur qui reste actif pour plusieurs parties en parallèle

## En cas de problème

### Port déjà utilisé :
```bash
lsof -i :5000
kill -9 [PID]
```

### Erreur de connexion :
- Vérifier que le serveur est lancé avant les clients
- Vérifier l'IP et le port (127.0.0.1:5000 par défaut)

### Problème de processus :
- Si des processus zombies apparaissent, vérifier que les processus enfants se terminent correctement
- Utiliser `ps aux | grep PN_serveur` pour voir les processus actifs

## Auteur

MOHAMMEDI Selyan
