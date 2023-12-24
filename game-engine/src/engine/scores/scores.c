#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../io.h"
#include "../scores.h"

#define SCORE_FILE_PATH "./scores.txt"

const static char *SCORE_DEFAULT = "sarra:100\nmondher:0\n";

// Fonction pour charger le fichier de scores
bool Score_load(void)
{
    File file_score = io_file_read(SCORE_FILE_PATH);
    if (!file_score.is_valid)
        return false;

    // printf("Fichier des scores :\n%s", file_score.data);

    free(file_score.data);

    return true;
}

// Fonction d'initialisation des scores
void local_score_init(void)
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

    fprintf(fp, "%s:%d\n", nom,score);

    fclose(fp);
}

// Fonction pour obtenir les scores locaux depuis le fichier
Score* GetLocalScores(size_t* count)
{
    // Lire le fichier des scores
    File file = io_file_read(SCORE_FILE_PATH);

    if (!file.is_valid)
    {
        fprintf(stderr, "Erreur lors de la lecture du fichier des scores.\n");
        return NULL;
    }

    char *token;
    char *delimiters = " \n";
    int num_scores = 0;

    
     // Copier la chaîne originale pour compter le nombre de scores
    char *temp_count_data = strdup(file.data);
    // Compter le nombre de scores pour allouer la mémoire
    char *temp_data = temp_count_data;
    while ((token = strtok(temp_data, delimiters)) != NULL)
    {
        num_scores++;
        temp_data = NULL;
    }
        // Libérer la mémoire de la copie utilisée pour compter
    free(temp_count_data);
    // Afficher le nombre de scores
    printf("Nombre de scores : %d\n", num_scores);

    // Allouer la mémoire pour le tableau de scores
    Score *scores = (Score *)malloc(num_scores * sizeof(Score));

    // Remplir le tableau de scores
    temp_data = file.data; 
   for (int i = 0; i < num_scores; i++)
    {
    //strtok utilisée pour découper une chaîne en sous-chaînes (tokens) en fonction de délimiteurs     
    // Utiliser NULL pour continuer le traitement de la même chaîne
        token = strtok(temp_data, ":");
        strncpy(scores[i].nom, token, sizeof(scores[i].nom) - 1);
        printf(" %d", i);     
        printf("Nom joueur = %s", scores[i].nom);
        printf(" %d", i);     
   // Utiliser NULL pour continuer le traitement de la même chaîne
        token = strtok(NULL, delimiters);

        // Vérifier si token n'est pas NULL avant d'extraire le score
        if (token != NULL)
        {
            scores[i].score = atoi(token);
            printf(" a fait un score de %d points\n", scores[i].score);
        }
        else
        {
            // Gérer l'erreur si le score est manquant
            fprintf(stderr, "Erreur: Score manquant pour le joueur %s\n", scores[i].nom);
            scores[i].score = 0;  // Définir le score à 0 par défaut
        }

        // Utiliser NULL pour continuer le traitement de la même chaîne
        temp_data = NULL;
    }

    // Libérer la mémoire du fichier lu
    free(file.data);

    // Mettre à jour le paramètre de sortie
    *count = num_scores;

    return scores;
}