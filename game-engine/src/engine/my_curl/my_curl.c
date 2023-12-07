
#include <stdlib.h>  // Pour malloc et free
#include <windows.h> // Pour le multithread

#include "../my_curl.h"
#include "../util.h"

#include "../render.h" // Ajout pour la fonction render_text
#include <string.h>
#include <stdio.h>


// Définir une structure pour stocker les données de la réponse
typedef struct
{
    char *data;
    size_t size;
} ResponseData;

void mycurl_init(MyCurlHandle *handle)
{
    // Initialiser libcurl
    handle->curl = curl_easy_init();

    if (!handle->curl)
    {
        ERROR_EXIT("\nErreur lors de l'initialisation de libcurl :  %s\n", stderr);
    }
}

// Fonction de rappel pour gérer les données de la réponse
static size_t mycurl_write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    printf("%.*s\n", (int)realsize, (char *)contents); // Imprimer les données de la réponse
    return realsize;
}

int mycurl_get(MyCurlHandle *handle, const char *endpoint)
{
    // Construire l'URL complet
    char url[256];
    snprintf(url, sizeof(url), "%s%s", SERVER_URL, endpoint);

    // Définir l'URL à requêter
    curl_easy_setopt(handle->curl, CURLOPT_URL, url);

    // Définir la fonction de rappel pour gérer les données de la réponse
    curl_easy_setopt(handle->curl, CURLOPT_WRITEFUNCTION, mycurl_write_callback);

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

// Fonction de rappel pour gérer les données de la réponse
static size_t mycurl_write_callback_code(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    ResponseData *data = (ResponseData *)userp;

    // Allouer de l'espace pour les nouvelles données
    data->data = realloc(data->data, data->size + realsize + 1);

    if (data->data == NULL)
    {
        // Échec de l'allocation de mémoire
        fprintf(stderr, "Erreur lors de l'allocation de mémoire\n");
        return 0;
    }

    // Copier les nouvelles données dans la structure
    memcpy(&(data->data[data->size]), contents, realsize);
    data->size += realsize;
    data->data[data->size] = '\0'; // Terminer la chaîne avec un caractère nul

    return realsize;
}

int mycurl_get(MyCurlHandle *handle, const char *endpoint)
{
    // Construire l'URL complet
    char url[256];
    snprintf(url, sizeof(url), "%s%s", SERVER_URL, endpoint);

    // Définir l'URL à requêter
    curl_easy_setopt(handle->curl, CURLOPT_URL, url);

    // Initialiser la structure pour stocker les données de la réponse
    ResponseData response_data = {NULL, 0};
    curl_easy_setopt(handle->curl, CURLOPT_WRITEDATA, &response_data);

    // Définir la fonction de rappel pour gérer les données de la réponse
    curl_easy_setopt(handle->curl, CURLOPT_WRITEFUNCTION, mycurl_write_callback);

    // Effectuer la requête
    CURLcode res = curl_easy_perform(handle->curl);

    // Vérifier les erreurs
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Erreur lors de la requête GET : %s\n", curl_easy_strerror(res));

        // Libérer la mémoire allouée pour les données de la réponse
        free(response_data.data);

        return 1; // Indique une erreur
    }

    // Afficher les données de la réponse avec render_text
    printf("Réponse du serveur : %s\n", response_data.data);

    // Libérer la mémoire allouée pour les données de la réponse
    free(response_data.data);

    return 0; // Succès
}
int mycurl_post(MyCurlHandle *handle, const char *endpoint, const char *post_data)
{
    // Construire l'URL complet
    char url[256];
    snprintf(url, sizeof(url), "%s%s", SERVER_URL, endpoint);

    // Définir l'URL à requêter
    curl_easy_setopt(handle->curl, CURLOPT_URL, url);

    // Définir la méthode de requête comme POST
    curl_easy_setopt(handle->curl, CURLOPT_POST, 1);

    // Données à envoyer avec la requête POST
    curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDS, post_data);

    // Effectuer la requête
    CURLcode res = curl_easy_perform(handle->curl);

    // Vérifier les erreurs
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Erreur lors de la requête POST : %s\n", curl_easy_strerror(res));
        return 1; // Indique une erreur
    }

    return 0; // Succès
}

void mycurl_cleanup(MyCurlHandle *handle)
{
    // Libérer les ressources de libcurl
    if (handle->curl)
    {
        curl_easy_cleanup(handle->curl);
    }
}

DWORD WINAPI async_curl_request(LPVOID data)
{
    CurlRequestData *curlData = (CurlRequestData *)data;

    // Effectuer la requête dans un thread séparé
    if (mycurl_get(curlData->handle, curlData->endpoint) != 0)
    {
        fprintf(stderr, "La requête GET vers %s a échoué.\n", curlData->endpoint);
    }

    free(curlData); // Libérer la mémoire allouée pour la structure

    return 0;
}