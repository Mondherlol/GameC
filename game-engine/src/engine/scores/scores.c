#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../io.h"
#include "../scores.h"
#include <curl/curl.h>
#include <windows.h>
#include "../my_curl.h"
#include "../util.h"
#include "../global.h"

#define SCORE_FILE_PATH "./scores.txt"

const static char *SCORE_DEFAULT = "sarra:60000\nmondher:2000000\n";

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
void local_score_init(void)
{
    if (Score_load())
    {
        printf("Scores déjà initialisés.\n");
        return;
    }

    // io_file_append((void *)SCORE_DEFAULT, strlen(SCORE_DEFAULT), SCORE_FILE_PATH);

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

    fprintf(fp, "%s:%d\n", nom, score);

    fclose(fp);
}

// Fonction utilitaire pour comparer deux scores pour le tri
static int compare_scores(const void *a, const void *b)
{
    return ((Score *)b)->score - ((Score *)a)->score;
}

// Fonction pour obtenir les scores locaux depuis le fichier

Score *GetLocalScores(size_t *count)
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
    // Allouer la mémoire pour le tableau de scores
    Score *scores = (Score *)malloc(num_scores * sizeof(Score));

    // Remplir le tableau de scores
    temp_data = file.data;

    for (int i = 0; i < num_scores; i++)
    {
        // strtok utilisée pour découper une chaîne en sous-chaînes (tokens) en fonction de délimiteurs
        //  Utiliser NULL pour continuer le traitement de la même chaîne
        token = strtok(temp_data, ":");
        strncpy(scores[i].nom, token, sizeof(scores[i].nom) - 1);
        // Utiliser NULL pour continuer le traitement de la même chaîne
        token = strtok(NULL, delimiters);

        // Vérifier si token n'est pas NULL avant d'extraire le score
        if (token != NULL)
        {
            scores[i].score = atoi(token);
        }
        else
        {
            // Gérer l'erreur si le score est manquant
            fprintf(stderr, "Erreur: Score manquant pour le joueur %s\n", scores[i].nom);
            scores[i].score = 0; // Définir le score à 0 par défaut
        }

        // Utiliser NULL pour continuer le traitement de la même chaîne
        temp_data = NULL;
    }

    // Libérer la mémoire du fichier lu
    free(file.data);

    // Tri décroissant des scores
    qsort(scores, num_scores, sizeof(Score), compare_scores);

    // Ne conserver que les 6 meilleurs scores
    if (num_scores > 6)
    {
        num_scores = 6;
    }

    // Mettre à jour le paramètre de sortie
    *count = num_scores;

    return scores;
}
// Fonction pour extraire les scores du JSON et retourner le nombre réel de scores extraits
static size_t extract_scores(const char *json, Score *onlinescores)
{
    const char *token = strtok(json, "[],\": \n\t\r");
    size_t score_count = 0;

    while (token != NULL && score_count < 6)
    {
        if (strcmp(token, "name") == 0)
        {
            token = strtok(NULL, "[],\": \n\t\r");
            strncpy(onlinescores[score_count].nom, token, sizeof(onlinescores[score_count].nom) - 1);
            onlinescores[score_count].nom[sizeof(onlinescores[score_count].nom) - 1] = '\0';
        }
        else if (strcmp(token, "score") == 0)
        {
            token = strtok(NULL, "[],\": \n\t\r");
            onlinescores[score_count].score = atoi(token);
            score_count++;
        }

        token = strtok(NULL, "[],\": \n\t\r");
    }

    return score_count;
}

// Fonction de rappel pour gérer les données de la réponse
static size_t my_curl_get_scores_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;

    // Convertir les données en une chaîne de caractères
    char *data = malloc(realsize + 1);
    if (data == NULL)
    {
        fprintf(stderr, "Erreur d'allocation de mémoire\n");
        return 0; // Arrêter le traitement en cas d'erreur
    }

    memcpy(data, contents, realsize);
    data[realsize] = '\0'; // Ajouter le caractère de fin de chaîne

    // Récupérer le tableau de scores passé en argument
    Score *onlinescores = (Score *)userp;

    // Extraire les scores du JSON
    size_t actual_scores = extract_scores(data, onlinescores, 6); // 6 est le nombre maximal de scores que vous pouvez traiter
    if (actual_scores != NULL)
    {
        for (size_t i = 0; i < actual_scores; i++)
        {
            char text[50];
            snprintf(text, sizeof(text), "%s: %d", onlinescores[i].nom, onlinescores[i].score);
            printf("%s\n", text);
        }
    }

    // Libérer la mémoire allouée
    free(data);

    return realsize;
}

static int my_curl_get_online_scores(MyCurlHandle *handle, const char *endpoint, Score *onlinescores)
{
    // Construire l'URL complet
    char url[256];
    snprintf(url, sizeof(url), "%s%s", SERVER_URL, endpoint);

    // Définir l'URL à requêter
    curl_easy_setopt(handle->curl, CURLOPT_URL, url);

    // Définir la fonction de rappel pour gérer les données de la réponse
    curl_easy_setopt(handle->curl, CURLOPT_WRITEFUNCTION, my_curl_get_scores_callback);

    // Passer le tableau onlinescores à la fonction de rappel
    curl_easy_setopt(handle->curl, CURLOPT_WRITEDATA, onlinescores);

    // Effectuer la requête
    CURLcode res = curl_easy_perform(handle->curl);

    // Vérifier les erreurs
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Erreur lors de la requête GET : %s\n", curl_easy_strerror(res));
        return 1; // Indique une erreur
    }
    return 0; // Succès
}

// Fonction qui sera exécutée dans un thread pour effectuer la requête CURL
DWORD WINAPI async_get_online_scores(LPVOID data)
{
    // Cast des données
    CurlRequestScoresData *curlData = (CurlRequestScoresData *)data;

    // Effectuer la requête dans un thread séparé
    if (my_curl_get_online_scores(&global.post_curl_handle, "/scores?limit=6", curlData->onlinescores) != 0)
    {
        curlData->onlinescores = NULL;
        fprintf(stderr, "La requête GET vers %s a échoué.\n", " /scores?limit=6");
        return 1; // Indique une erreur
    }

    return 0;
}
