## Ouvrir le jeu

1. Afficher le menu
2. Générer et stocker un code de partie via une requête GET
   - Si succès, afficher le code de la partie en haut à gauche 
   - En cas d'échec, afficher "Non connecté"

## Jouer

1. Afficher l'écran de jeu
2. Le joueur joue en tuant des ennemis, ramassant des collectibles, et augmentant son score
3. Les joueurs peuvent rejoindre la partie et ajouter des ennemis en cours de jeu
4. Les  pseudos des joueurs sont affichés au dessus des ennemeis qu'ils ont choisis.

## Quand le joueur perd (Game Over)

- Saisir le pseudo pour enregistrer le score
  - Afficher par qui l'ennemi a ete envoyé 
- Si le joueur saisit un pseudo, envoyer son score à la base de données et enregistrer localement

### Options après la défaite :

- **Rejouer**
  - Oui : Rejouer
  - Non : Retourner au menu

## Scores

- Afficher les 10 meilleurs scores personnels, a charger depuis un fichier local
- Afficher le High Score mondial avec requete GET au serveur

## Quitter

- Fermer le jeu