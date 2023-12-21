// Structure score nom, score

// WriteLocalScore(nom, score) -> Crée un fichier dans le dossier du jeu avec tous les scores locaux

// GetLocalScore() -> Retourne Tableau de structure score contenant les score locaux

// SendOnlineScore(nom, score) -> requete post a la base de donnee

// GetOnlineScore() -> Retourne Tableau de structure score contenant les score de la base

#pragma once

// Structure représentant un score avec un nom et une valeur de score
typedef struct
{
    char *nom;
    int score;
} Score;

// Fonction pour écrire un score dans le fichier des scores locaux
void WriteLocalScore(const char *nom, int score);

// Fonction pour obtenir les scores locaux depuis le fichier
Score *GetLocalScore();
