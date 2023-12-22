// Structure score nom, score

// WriteLocalScore(nom, score) -> Crée un fichier dans le dossier du jeu avec tous les scores locaux

// GetLocalScore() -> Retourne Tableau de structure score contenant les score locaux

// SendOnlineScore(nom, score) -> requete post a la base de donnee

// GetOnlineScore() -> Retourne Tableau de structure score contenant les score de la base

#pragma once
#include "io.h"
#include <stdbool.h>

// Structure représentant un score avec un nom et une valeur de score
typedef struct
{
    char nom[50];
    int score;
} Score;

// Crée un fichier dans le dossier du jeu avec tous les scores locaux
void WriteLocalScore(const char *nom, int score);

// Retourne true si la lecture des scores est réussie, false sinon
bool Score_load(void);

// Fonction d'initialisation des scores
void score_init(void);
// Retourne un tableau de structure Score contenant les scores locaux
Score* GetLocalScores(size_t* count);