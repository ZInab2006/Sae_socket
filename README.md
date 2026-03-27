# JEU DU PENDU - Version 3 (V3)

## Description

Version 3 du jeu du pendu en réseau. Cette version permet à **2 joueurs** de jouer ensemble, avec la possibilité pour le serveur de gérer **plusieurs parties simultanément** grâce à l'utilisation de `fork()`.

- **Client unique** : Un seul fichier client qui gère automatiquement les deux rôles selon les codes reçus
- **Client 1** : Le joueur qui fait deviner (entre le mot secret)
- **Client 2** : Le joueur qui devine (propose des lettres)

Le **serveur** fait transiter les messages entre les 2 clients et utilise des processus séparés (`fork()`) pour gérer chaque partie en parallèle.

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
- ✅ **Client unique** : Un seul fichier client pour les deux rôles (détection automatique)
- ✅ **Meilleure gestion des connexions** : Le serveur principal reste disponible pour de nouvelles parties

## Compilation

### Linux/macOS :
```bash
# Compiler le serveur
gcc -o PN_serveur_V3 PN_serveur_V3.c

# Compiler le client unique
gcc -o PN_client_V3 PN_client_V3.c
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
./PN_client_V3 127.0.0.1 5000
```
Le client 1 reçoit automatiquement le code `2001` (JOUEUR_1_ENTRE_MOT) et doit entrer un mot à faire deviner.

### Terminal 3 - Client 2 (devine) :
```bash
./PN_client_V3 127.0.0.1 5000
```
Le client 2 reçoit automatiquement le code `2002` (JOUEUR_2_TAILLE_MOT_ET_PEUT_JOUER) et peut commencer à proposer des lettres.

**Note** : Vous pouvez lancer plusieurs paires de clients simultanément, le serveur gérera chaque partie dans un processus séparé grâce à `fork()`.

## Règles du jeu

- Le **Client 1** entre un mot secret (maximum 50 caractères)
- Le **Client 2** doit deviner le mot en proposant des lettres
- Maximum d'erreurs : **6**
- Le pendu s'affiche progressivement avec chaque erreur
- La partie se termine quand :
  - Le mot est complètement découvert → **Victoire**
  - 6 erreurs sont commises → **Défaite**

## Format des messages (codes)

Le système utilise des codes numériques pour identifier le type de message :

### Codes de communication :
- **2001** : `JOUEUR_1_ENTRE_MOT` - Le serveur demande au Client 1 d'entrer un mot
- **2002** : `JOUEUR_2_TAILLE_MOT_ET_PEUT_JOUER` - Le Client 2 reçoit la taille du mot et peut commencer
- **2003** : `JOUEUR_2_PROPOSE_LETTRE` - Le Client 2 propose une lettre
- **2004** : `JOUEUR_1_VALIDE_OU_NON` - Le Client 1 valide ou non la lettre
- **2005** : `JOUEUR_2_RECOIT_VALIDE_OU_NON` - Le Client 2 reçoit la validation
- **2006** : `JOUEUR_2_DONNEES_PARTIE` - Le Client 2 reçoit les données de la partie (mot, erreurs, statut)
- **3001** : `MESSAGE` - Message informatif (ne déclenche aucune action)

### Format des messages :
Les messages sont formatés comme suit : `code:données`

Exemples :
- `2001:` → Demande au Client 1 d'entrer un mot
- `2002:5` → Client 2 reçoit la taille du mot (5 lettres)
- `2003:P` → Client 2 propose la lettre P
- `2006:oui P__DU 0` → Client 2 reçoit les données (oui, mot partiel, 0 erreur)

## Fonctionnalités

- ✅ Communication TCP/IP avec système de codes
- ✅ Gestion multi-parties simultanées avec `fork()`
- ✅ Client unique pour les deux rôles (détection automatique)
- ✅ Affichage du pendu ASCII selon le nombre d'erreurs
- ✅ Gestion des lettres déjà testées
- ✅ Détection automatique de fin de partie
- ✅ Serveur qui reste actif pour plusieurs parties

## Organigramme

Un organigramme détaillé de la communication est disponible dans `organigramme_V3.svg`.

## En cas de problème

### Port déjà utilisé :
```bash
lsof -i :5000
kill -9 [PID]
```

### Erreur de connexion :
- Vérifier que le serveur est lancé avant les clients
- Vérifier l'IP et le port (127.0.0.1:5000 par défaut)

### Problème avec fork() :
- Vérifier que le système supporte `fork()` (Linux/macOS)
- Sur Windows, utiliser WSL ou MinGW

## Auteur

MOHAMMEDI Selyan
