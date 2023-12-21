#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../io.h" 

#define SCORE_FILE_PATH "./scores.txt"

const static char *SCORE_DEFAULT =
    "sarra:100\n"
    "mondher:0\n"
    "\n";
;

//pour charger le fichier score
// static int Score_load(void)
// {
//     File file_score = io_file_read("./scores.txt");
//     if (!file_score.is_valid)
//         return 1;

//     printf("fichier : %s", file_score.data);

//     free(file_score.data);

//     return 0;
// }


// void score_init(void)
// {
//     if (Score_load() == 0)
//         return;

//     io_file_append((void *)SCORE_DEFAULT, strlen(SCORE_DEFAULT), "./scores.txt");

//     if (Score_load() != 0)
//         ERROR_EXIT("Impossible de charger ou creer scores.txt.\n");
// }

// Fonction pour ajouter un nouveau score au fichier des scores
// void WriteLocalScore(const char *nom, int score)
// {
//     print("writing local score")
//     FILE *fp = fopen(SCORE_FILE_PATH, "a");
//     if (!fp || ferror(fp))
//     {
//         fprintf(stderr, "Erreur lors de l'ouverture du fichier des scores en écriture.\n");
//         return;
//     }

//     // Écrire le nouveau score dans le fichier
//     fprintf(fp, "%s %d\n", nom, score);
    
//     fclose(fp);
// }

// Fonction pour obtenir les scores locaux depuis le fichier
// Score* GetLocalScore()
// {
//     // Lire le fichier des scores
//     File file = io_file_read(SCORE_FILE_PATH);

//     if (!file.is_valid)
//     {
//         // Si le fichier n'existe pas, créez-le et réessayez la lecture
//         FILE *createFile = fopen(SCORE_FILE_PATH, "w");
//         if (!createFile || ferror(createFile))
//         {







    
//             fprintf(stderr, "Erreur lors de la création du fichier des scores.\n");
//             return NULL;
//         }
//         fclose(createFile);

//         // Réessayez la lecture après avoir créé le fichier
//         file = io_file_read(SCORE_FILE_PATH);
//         if (!file.is_valid)
//         {
//             fprintf(stderr, "Erreur lors de la lecture du fichier des scores après création.\n");
//             return NULL;
//         }
//     }

//     char *token;
//     char *delimiters = " \n";
//     char *saveptr;
//     int num_scores = 0;

//     // Compter le nombre de scores pour allouer la mémoire
//     char *temp_data = file.data;
//     while ((token = strtok_r(temp_data, delimiters, &saveptr)) != NULL)
//     {
//         num_scores++;
//         temp_data = NULL;
//     }

//     // Allouer la mémoire pour le tableau de scores
//     Score *scores = (Score *)malloc(num_scores * sizeof(Score));

//     // Remplir le tableau de scores
//     temp_data = file.data;
//     for (int i = 0; i < num_scores; i++)
//     {
//         scores[i].nom = strdup(strtok_r(temp_data, delimiters, &saveptr));
//         temp_data = NULL;
//         scores[i].score = atoi(strtok_r(temp_data, delimiters, &saveptr));
//         temp_data = NULL;
//     }

//     // Libérer la mémoire du fichier lu
//     free(file.data);

//     return scores;
// }
