# 🧩 SAE_Socket — Jeu du Pendu en C avec Sockets

## 🎯 Description du projet

Le projet **SAE_Socket** a été réalisé dans le cadre d’une **SAÉ (Situation d’Apprentissage et d’Évaluation)** à l’IUT.  
L’objectif principal est de concevoir **plusieurs versions évolutives** d’une application client-serveur en **langage C**, en utilisant **les sockets TCP/IP** pour la communication entre les processus.

Chaque version introduit de **nouvelles fonctionnalités** et une **meilleure architecture** réseau, en s’appuyant sur le même concept de base : le **jeu du Pendu**.

---

## 🧠 Objectifs pédagogiques

- Comprendre le **fonctionnement des sockets** en C (communication réseau bas niveau).  
- Implémenter une architecture **client/serveur**.  
- Gérer **les échanges et la synchronisation** entre plusieurs processus.  
- Améliorer progressivement le code (**robustesse, modularité, expérience utilisateur**).  

---

## ⚙️ Fonctionnement global

Le projet se compose de deux programmes principaux :

- **Serveur** :  
  Gère le mot à deviner, la connexion des clients, et les échanges réseau.  
  Il renvoie les réponses et l’état du jeu à chaque tentative du joueur.

- **Client** :  
  Se connecte au serveur, envoie les lettres à deviner, et affiche les retours côté joueur.

---

## 🚀 Versions développées

| Version | Description | Améliorations clés |
|----------|--------------|--------------------|
| **v1 — Connexion simple** | Création d’un serveur et d’un client basique avec des sockets TCP. | Envoi/réception de messages simples. |
| **v2 — Jeu du pendu intégré** | Intégration de la logique du jeu du pendu côté serveur. | Gestion d’un seul joueur. |
| **v3 — Multi-clients** | Le serveur gère plusieurs clients simultanément. | Utilisation de `select()` ou `fork()`. |
| **v4 — Améliorations réseau** | Amélioration de la robustesse et de la modularité du code. | Meilleure gestion des erreurs, découpage en modules. |

---

## 🧩 Architecture du dépôt

SAE_Socket/
├── src/
│ ├── serveur/
│ │ ├── serveur_v1.c
│ │ ├── serveur_v2.c
│ │ ├── ...
│ ├── client/
│ │ ├── client_v1.c
│ │ ├── client_v2.c
│ │ ├── ...
├── include/
│ ├── fonctions.h
│ ├── constants.h
├── docs/
│ ├── compte_rendu.pdf
│ ├── diagrammes/
└── README.md

---

## 🧪 Compilation et exécution

### Compilation
Utiliser `make` pour compiler les différentes versions :
make serveur_v2
make client_v2

### Exécution
Démarrer d’abord le serveur :
./serveur_v2
Puis le client dans un autre terminal :
./client_v2

---

## 👥 Équipe de développement

Projet réalisé par :  
- **[OUTMANI Zinab]** — [S'occupe de la version n°]  
- **[KIME Marwa]** — [S'occupe de la version n°]  
- **[GOBFERT Frédéric]** — [S'occupe de la version n°]
- **[MOHAMMEDI Selyan]** — [S'occupe de la version n°]  


> Encadré par **[M.François Rousselle]**, Département Informatique — IUT de Calais.

---

## 📚 Technologies utilisées

- **Langage** : C pur  
- **Protocoles** : TCP/IP (sockets POSIX)  
- **Outils** : GCC, Makefile, Git, Linux terminal  

---

## 🧾 Licence

Projet académique — utilisation libre à des fins pédagogiques.

---
