# JEU DU PENDU - Version 1 (V1)

## Description

Version 1 du jeu du pendu en réseau. Cette version permet à **2 joueurs** de jouer ensemble en **alternance** (tour par tour) pour deviner un mot secret.

- **Client 1** : Premier joueur à se connecter, attend le deuxième joueur
- **Client 2** : Deuxième joueur, démarre la partie
- **Mot secret** : Fixe, défini dans le code serveur (`"PENDU"`)

Le **serveur** gère la logique du jeu, alterne les tours entre les deux joueurs, et gère l'élimination si un joueur atteint le maximum d'erreurs.

## Architecture

```
Client 1  ←→  Serveur (gère la partie)  ←→  Client 2
                ↓
          [Tour par tour]
          [Gestion élimination]
```

## Caractéristiques principales

- ✅ **Jeu à 2 joueurs** : Deux clients connectés simultanément
- ✅ **Tour par tour** : Les joueurs jouent en alternance
- ✅ **Système d'élimination** : Si un joueur atteint 6 erreurs, il est éliminé et l'autre continue
- ✅ **Affichage du pendu ASCII** : Visualisation progressive des erreurs
- ✅ **Interface claire** : Effacement d'écran et affichage structuré
- ✅ **Messages formatés** : Communication avec séparateurs (`|` et `$`)

## Compilation

### Linux/macOS :
```bash
# Compiler le serveur
gcc -o PN_serveur_V1 PN_serveur_V1.c

# Compiler le client
gcc -o PN_client_V1 PN_client_V1.c
```

### Windows (avec MinGW ou WSL) :
Même commande dans WSL ou MinGW

## Exécution

### Terminal 1 - Serveur :
```bash
./PN_serveur_V1
```

### Terminal 2 - Client 1 (premier joueur) :
```bash
./PN_client_V1 127.0.0.1 5000
```
Le Client 1 se connecte et attend qu'un deuxième joueur arrive.

### Terminal 3 - Client 2 (deuxième joueur) :
```bash
./PN_client_V1 127.0.0.1 5000
```
Le Client 2 se connecte et la partie commence immédiatement.

## Règles du jeu

- Le **mot secret** est fixe : `"PENDU"` (5 lettres)
- Maximum d'erreurs par joueur : **6**
- **Tour par tour** : Un joueur joue pendant que l'autre attend
- **Élimination** : Si un joueur atteint 6 erreurs, il est éliminé et l'autre continue seul
- La partie se termine quand :
  - Un joueur trouve le mot complet → **Victoire pour ce joueur**
  - Les deux joueurs sont éliminés → **Défaite pour les deux**

## Format des messages

Le système utilise un format de message structuré avec des séparateurs :

### Format général :
```
ENTETE|VALEUR$MESSAGE
```

- **ENTETE** : Type de message (`INIT`, `PLAY`, `WAIT`, `FIN`)
- **VALEUR** : Paramètre numérique (nombre d'erreurs, longueur du mot)
- **MESSAGE** : Texte à afficher

### Types de messages :

#### INIT
- **Envoyé au Client 1** : `INIT|5$Vous êtes connecté. Veuillez attendre...`
- **Envoyé au Client 2** : `INIT|5$La partie commence !`
- **Action** : Initialise la partie avec la longueur du mot

#### PLAY
- **Format** : `PLAY|X$\nA vous de jouer !\n\n\tMOT_ACTUEL`
- **Action** : C'est le tour du joueur de proposer une lettre

#### WAIT
- **Format** : `WAIT|X$\nEn attente, joueur Y joue !\n\n\tMOT_ACTUEL`
- **Action** : Le joueur attend son tour

#### FIN
- **Format** : `FIN|X$\nPARTIE TERMINÉE\n\n...`
- **Action** : Fin de partie (victoire ou défaite)

### Réponses aux lettres proposées :

Le serveur répond avec un format simple : `STATUS MOT_ACTUEL ERREURS`

- **`oui MOT_ACTUEL ERREURS`** : Lettre trouvée dans le mot
- **`non MOT_ACTUEL ERREURS`** : Lettre absente du mot
- **`deja MOT_ACTUEL ERREURS`** : Lettre déjà proposée
- **`erreur MOT_ACTUEL ERREURS`** : Caractère invalide
- **`gagne MOT_ACTUEL ERREURS`** : Le joueur a gagné
- **`perdu MOT_SECRET ERREURS`** : Le joueur a perdu
- **`elimine MOT_SECRET ERREURS`** : Le joueur est éliminé (6 erreurs)

## Fonctionnalités

- ✅ Communication TCP/IP avec messages formatés
- ✅ Gestion de deux clients simultanés
- ✅ Alternance des tours entre joueurs
- ✅ Système d'élimination (joueur éliminé si 6 erreurs)
- ✅ Affichage du pendu ASCII selon le nombre d'erreurs
- ✅ Gestion des lettres déjà testées
- ✅ Interface utilisateur avec effacement d'écran
- ✅ Détection automatique de fin de partie

## Déroulement d'une partie

1. **Client 1 se connecte** → Reçoit `INIT` et attend
2. **Client 2 se connecte** → Reçoit `INIT` et la partie commence
3. **Tour du Client 2** :
   - Reçoit `PLAY` → Propose une lettre
   - Reçoit réponse (`oui`, `non`, etc.)
4. **Tour du Client 1** :
   - Reçoit `PLAY` → Propose une lettre
   - Reçoit réponse
5. **Alternance** jusqu'à :
   - Un joueur trouve le mot → **Victoire**
   - Un joueur atteint 6 erreurs → **Élimination**, l'autre continue
   - Les deux joueurs sont éliminés → **Défaite**

## En cas de problème

### Port déjà utilisé :
```bash
lsof -i :5000
kill -9 [PID]
```

### Erreur de connexion :
- Vérifier que le serveur est lancé avant les clients
- Vérifier l'IP et le port (127.0.0.1:5000 par défaut)
- Le Client 1 doit se connecter avant le Client 2

### Problème d'affichage :
- Vérifier que le terminal supporte les codes ANSI (clear screen)
- Sur certains terminaux, l'effacement d'écran peut ne pas fonctionner

## Différences avec V0

- **V0** : Un seul joueur contre le serveur, mot fixe
- **V1** : Deux joueurs en alternance, système d'élimination, messages formatés, affichage amélioré

## Auteur

Karadoc
