# JEU DU PENDU - Version 2 (V2)

## Description

Version 2 du jeu du pendu en réseau. Cette version permet à **2 joueurs** de jouer ensemble :
- **Client 1** : Le joueur qui fait deviner (entre le mot secret)
- **Client 2** : Le joueur qui devine (propose des lettres)

Le **serveur** fait uniquement transiter les messages entre les 2 clients sans intervenir dans la logique du jeu.

## Architecture

```
Client 1 (fait deviner)  ←→  Serveur (relais)  ←→  Client 2 (devine)
```

## Compilation

### Linux/macOS :
```bash
# Compiler le serveur
gcc -o PN_serveur_V2 PN_serveur_V2.c

# Compiler les clients
gcc -o PN_client1_V2 PN_client1_V2.c
gcc -o PN_client2_V2 PN_client2_V2.c
```

### Windows (avec MinGW ou WSL) :
Même commande dans WSL ou MinGW

## Exécution

### Terminal 1 - Serveur :
```bash
./PN_serveur_V2
```

### Terminal 2 - Client 1 (fait deviner) :
```bash
./PN_client1_V2
```
Le client 1 doit entrer un mot à faire deviner.

### Terminal 3 - Client 2 (devine) :
```bash
./PN_client2_V2 127.0.0.1 5000
```
Le client 2 propose des lettres pour deviner le mot.

## Règles du jeu

- Le **Client 1** entre un mot secret (maximum 50 caractères)
- Le **Client 2** doit deviner le mot en proposant des lettres
- Maximum d'erreurs : **6**
- Le pendu s'affiche progressivement avec chaque erreur
- La partie se termine quand :
  - Le mot est complètement découvert → **Victoire**
  - 6 erreurs sont commises → **Défaite**

## Format des messages

### Messages envoyés par le serveur :
- `"Veuillez entrer un mot à deviner : "` → Client 1
- `"start x"` → Client 2 (x = nombre de lettres)

### Messages entre clients (via le serveur) :
- Client 2 → Client 1 : Lettre proposée (ex: `"P"`)
- Client 1 → Client 2 : Réponse formatée
  - `"oui mot_decouvert nb_erreurs"` (ex: `"oui P__DU 0"`)
  - `"non mot_decouvert nb_erreurs"` (ex: `"non P__DU 1"`)
  - `"deja mot_decouvert nb_erreurs"` (lettre déjà testée)
  - `"gagne mot_complet nb_erreurs"` (victoire)
  - `"perdu mot_secret nb_erreurs"` (défaite)

## Fonctionnalités

- ✅ Communication TCP/IP entre 2 clients via un serveur
- ✅ Affichage du pendu ASCII selon le nombre d'erreurs
- ✅ Gestion des lettres déjà testées
- ✅ Détection automatique de fin de partie
- ✅ Serveur qui reste actif pour plusieurs parties

## En cas de problème

### Port déjà utilisé :
```bash
lsof -i :5000
kill -9 [PID]
```

### Erreur de connexion :
- Vérifier que le serveur est lancé avant les clients
- Vérifier l'IP et le port (127.0.0.1:5000 par défaut)

## Auteur

Mapbaya

