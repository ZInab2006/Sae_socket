# JEU DU PENDU - Version 4 (V4)

## Description

Version 4 du jeu du pendu en réseau. Cette version permet à **2 joueurs** de jouer ensemble en **communication peer-to-peer (P2P)** après une phase de **matchmaking** par un serveur central.

- **Serveur de matchmaking** : Met en relation deux clients, puis se retire
- **Client 1** : Devient serveur P2P et fait deviner le mot
- **Client 2** : Se connecte en P2P et devine le mot

La communication du jeu se fait **directement entre les clients** sans passer par le serveur de matchmaking.

## Architecture

```
Phase 1 - Matchmaking :
Client 1  ←→  Serveur (matchmaking)  ←→  Client 2

Phase 2 - Communication P2P :
Client 1 (serveur P2P)  ←→  Client 2 (client P2P)
```

## Améliorations par rapport à V3

- ✅ **Communication P2P** : Les clients communiquent directement après le matchmaking
- ✅ **Serveur allégé** : Le serveur ne fait que le matchmaking, pas de relais de messages
- ✅ **Meilleure scalabilité** : Le serveur peut gérer plusieurs paires de joueurs simultanément
- ✅ **Architecture décentralisée** : Chaque partie est autonome une fois le matchmaking terminé

## Compilation

### Linux/macOS :
```bash
# Compiler le serveur de matchmaking
gcc -o PN_serveur_V4 PN_serveur_V4.c

# Compiler le client
gcc -o PN_client_V4 PN_client_V4.c
```

### Windows (avec MinGW ou WSL) :
Même commande dans WSL ou MinGW

## Exécution

### Terminal 1 - Serveur de matchmaking :
```bash
./PN_serveur_V4
```
Le serveur écoute sur le port **5000** et attend deux clients.

### Terminal 2 - Client 1 (devient serveur P2P) :
```bash
./PN_client_V4 127.0.0.1 5000
```
Le client 1 :
- Se connecte au serveur de matchmaking
- Reçoit le rôle "SERVEUR" et le port 6000
- Crée un serveur P2P sur le port 6000
- Attend le Client 2
- Fait deviner le mot

### Terminal 3 - Client 2 (devient client P2P) :
```bash
./PN_client_V4 127.0.0.1 5000
```
Le client 2 :
- Se connecte au serveur de matchmaking
- Reçoit le rôle "CLIENT", l'IP du Client 1 et le port 6000
- Se connecte au serveur P2P (Client 1)
- Devine le mot

**Note** : Le rôle (SERVEUR ou CLIENT) est automatiquement attribué par le serveur de matchmaking selon l'ordre de connexion.

## Règles du jeu

- Le **Client 1 (serveur P2P)** entre un mot secret (maximum 50 caractères)
- Le **Client 2 (client P2P)** doit deviner le mot en proposant des lettres
- Maximum d'erreurs : **6**
- Le pendu s'affiche progressivement avec chaque erreur
- La partie se termine quand :
  - Le mot est complètement découvert → **Victoire**
  - 6 erreurs sont commises → **Défaite**

## Format des messages

### Phase de matchmaking (serveur central) :
- Serveur → Client 1 : `"SERVEUR 6000"` (Client 1 devient serveur P2P)
- Serveur → Client 2 : `"CLIENT [ip_client1] 6000"` (Client 2 se connecte en P2P)

### Phase P2P (communication directe) :
- Client 1 → Client 2 : `"start x"` (x = nombre de lettres)
- Client 2 → Client 1 : Lettre proposée (ex: `"P"`)
- Client 1 → Client 2 : Réponse formatée
  - `"oui mot_decouvert nb_erreurs"` (ex: `"oui P__DU 0"`)
  - `"non mot_decouvert nb_erreurs"` (ex: `"non P__DU 1"`)
  - `"deja mot_decouvert nb_erreurs"` (lettre déjà testée)
  - `"erreur mot_decouvert nb_erreurs"` (caractère invalide)
  - `"gagne mot_complet nb_erreurs"` (victoire)
  - `"perdu mot_secret nb_erreurs"` (défaite)

## Fonctionnalités

- ✅ Communication TCP/IP avec matchmaking initial
- ✅ Communication P2P directe entre les clients
- ✅ Affichage du pendu ASCII selon le nombre d'erreurs
- ✅ Gestion des lettres déjà testées
- ✅ Détection automatique de fin de partie
- ✅ Serveur de matchmaking qui reste actif pour plusieurs parties
- ✅ Architecture décentralisée (chaque partie est autonome)

## Organigramme

Un organigramme détaillé de la communication est disponible dans `ModeleV4.png`.

## En cas de problème

### Port déjà utilisé :
```bash
# Port 5000 (matchmaking)
lsof -i :5000
kill -9 [PID]

# Port 6000 (P2P)
lsof -i :6000
kill -9 [PID]
```

### Erreur de connexion :
- Vérifier que le serveur de matchmaking est lancé avant les clients
- Vérifier l'IP et le port (127.0.0.1:5000 par défaut)
- Vérifier que le firewall n'bloque pas les ports 5000 et 6000

### Problème de connexion P2P :
- Si les clients sont sur des machines différentes, vérifier que le Client 1 (serveur P2P) est accessible depuis le Client 2
- Vérifier les règles de firewall pour le port 6000
- Vérifier que l'IP envoyée par le serveur est correcte (peut être un problème avec NAT)

## Auteur

Gobfert Frédéric
