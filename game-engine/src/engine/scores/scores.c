#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../io.h"
#include "../scores.h"

#define SCORE_FILE_PATH "C:/Users/PC/Desktop/projetgameC/GameC/game-engine/src/engine/scores/scores.txt"

const static char *SCORE_DEFAULT = "sarra:100\nmondher:0\n";

// Fonction pour charger le fichier de scores
bool Score_load(void)
{
    File file_score = io_file_read(SCORE_FILE_PATH);
    if (!file_score.is_valid)
        return false;

    printf("Fichier des scores :\n%s", file_score.data);

    free(file_score.data);

    return true;
}

// Fonction d'initialisation des scores
void Score_init(void)
{
     if (Score_load())
    {
        printf("Scores déjà initialisés.\n");
        return ;
    }

    io_file_append((void *)SCORE_DEFAULT, strlen(SCORE_DEFAULT), SCORE_FILE_PATH);

     if (Score_load())
    {
        printf("Initialisation des scores réussie.\n");
    }
    else
    {
        fprintf(stderr, "Impossible de charger ou créer scores.txt.\n");
    }
}

// Crée un fichier dans le dossier du jeu avec tous les scores locaux
void WriteLocalScore(const char *nom, int score)
{
    FILE *fp = fopen(SCORE_FILE_PATH, "a");
    if (!fp)
    {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier des scores : %s.\n", SCORE_FILE_PATH);
        return;
    }

    fprintf(fp, "%s,%d\n", nom, score);

    fclose(fp);
}

// Fonction pour obtenir les scores locaux depuis le fichier
Score *GetLocalScores(size_t *count)
{
    // Read the scores file
    File file = io_file_read(SCORE_FILE_PATH);

    if (!file.is_valid)
    {
        fprintf(stderr, "Erreur lors de la lecture du fichier des scores : %s.\n", SCORE_FILE_PATH);
        return NULL;
    }

    char *token;
    char *delimiters = " \n";
    char *saveptr;

    // Count the number of scores to allocate memory
    char *temp_data = file.data;
    while ((token = strtok(temp_data, delimiters)) != NULL)
    {
        (*count)++;
        temp_data = NULL;
    }

    // Allocate memory for the scores array
    Score *scores = (Score *)malloc((*count) * sizeof(Score));

    // Fill the scores array
    temp_data = file.data;
    for (size_t i = 0; i < (*count); i++)
    {
        strncpy(scores[i].nom, strtok(temp_data, delimiters), sizeof(scores[i].nom) - 1);
        temp_data = NULL;
        scores[i].score = atoi(strtok(temp_data, delimiters));
        temp_data = NULL;
    }

    // Free the memory of the read file
    free(file.data);

    // Display the scores in the console
    for (size_t i = 0; i < (*count); i++)
    {
        printf("Nom: %s, Score: %d\n", scores[i].nom, scores[i].score);
    }

    return scores;
}