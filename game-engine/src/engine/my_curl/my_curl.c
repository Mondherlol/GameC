
#include <stdlib.h>  // Pour malloc et free
#include <windows.h> // Pour le multithread
#include "../my_curl.h"
#include "../util.h"
#include "../global.h"

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
    snprintf(url, sizeof(url), "%s%s", global.SERVER_URL, endpoint);

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

int mycurl_post(MyCurlHandle *handle, const char *endpoint, const char *post_data)
{
    // Construire l'URL complet
    char url[256];
    snprintf(url, sizeof(url), "%s%s", global.SERVER_URL, endpoint);

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

// Fonction de rappel pour gérer les données de la réponse
static size_t callback_post_response(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    AsyncCallback callback = (AsyncCallback)userp;
    callback((const char *)contents);
    return realsize;
}

DWORD WINAPI async_curl_post_request(LPVOID data)
{
    CurlPostRequestData *postRequestData = (CurlPostRequestData *)data;

    // Définir l'URL à requêter
    char url[256];
    snprintf(url, sizeof(url), "%s%s", global.SERVER_URL, postRequestData->endpoint);

    // Définir l'URL à requêter
    curl_easy_setopt(postRequestData->handle->curl, CURLOPT_URL, url);

    // Définir la méthode de requête comme POST
    curl_easy_setopt(postRequestData->handle->curl, CURLOPT_POST, 1);

    // Données à envoyer avec la requête POST
    curl_easy_setopt(postRequestData->handle->curl, CURLOPT_POSTFIELDS, postRequestData->post_data);

    // Définir la fonction de rappel pour gérer les données de la réponse
    curl_easy_setopt(postRequestData->handle->curl, CURLOPT_WRITEFUNCTION, callback_post_response);
    curl_easy_setopt(postRequestData->handle->curl, CURLOPT_WRITEDATA, postRequestData->callback_function);

    // Effectuer la requête
    CURLcode res = curl_easy_perform(postRequestData->handle->curl);

    // Vérifier les erreurs
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Erreur lors de la requête POST : %s\n", curl_easy_strerror(res));
        postRequestData->callback_function("Error");
    }

    free(postRequestData->endpoint);  // Libérer la mémoire allouée pour l'endpoint
    free(postRequestData->post_data); // Libérer la mémoire allouée pour les données POST
    free(postRequestData);            // Libérer la mémoire allouée pour la structure

    return 0;
}

// Fonction pour effectuer une requête POST de manière asynchrone
void mycurl_post_async(MyCurlHandle *handle, const char *endpoint, const char *post_data, AsyncCallback callback_function)
{
    // Allouer de la mémoire pour la structure de données de requête POST
    CurlPostRequestData *postRequestData = (CurlPostRequestData *)malloc(sizeof(CurlPostRequestData));
    if (!postRequestData)
    {
        fprintf(stderr, "Erreur lors de l'allocation de mémoire pour postRequestData\n");
    }

    // Allouer de la mémoire pour copier l'endpoint et les données POST
    postRequestData->endpoint = _strdup(endpoint);
    postRequestData->post_data = _strdup(post_data);

    // Vérifier si l'allocation de mémoire a réussi
    if (!postRequestData->endpoint || !postRequestData->post_data)
    {
        fprintf(stderr, "Erreur lors de l'allocation de mémoire pour l'endpoint ou les données POST\n");
        free(postRequestData->endpoint);
        free(postRequestData->post_data);
        free(postRequestData);
    }

    postRequestData->handle = handle;
    postRequestData->callback_function = callback_function;

    // Démarrer un thread pour effectuer la requête POST de manière asynchrone
    HANDLE thread_handle = CreateThread(NULL, 0, async_curl_post_request, postRequestData, 0, NULL);
    if (thread_handle == NULL)
    {
        fprintf(stderr, "Erreur lors de la création du thread pour la requête POST asynchrone\n");
        free(postRequestData->endpoint);
        free(postRequestData->post_data);
        free(postRequestData);
    }

    // // Attendre la fin du thread
    // WaitForSingleObject(thread_handle, INFINITE);
    CloseHandle(thread_handle);
}
