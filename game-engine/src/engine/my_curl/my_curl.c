
#include <stdlib.h>  // Pour malloc et free
#include <windows.h> // Pour le multithread

#include "../my_curl.h"
#include "../util.h"

#include "../render.h" // Ajout pour la fonction render_text




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

// Variable globale pour stocker le code reçu
char *receivedCode = NULL;

// Fonction de rappel pour gérer les données de la réponse contenant le code
static size_t mycurl_generate_code(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    
    // Allouer de la mémoire pour stocker la chaîne de code
    receivedCode = (char *)malloc(realsize + 1);
    if (receivedCode)
    {
        // Copier les données de la réponse dans la chaîne de code
        memcpy(receivedCode, contents, realsize);
        receivedCode[realsize] = '\0'; // Ajouter la terminaison de la chaîne de caractères
    }
    return realsize;
}


char *mycurl_get_code(MyCurlHandle *handle, const char *endpoint)
{
    // Construire l'URL complet
    char url[256];
    snprintf(url, sizeof(url), "%s%s", SERVER_URL, endpoint);

    // Définir l'URL à requêter
    curl_easy_setopt(handle->curl, CURLOPT_URL, url);

    // Définir la fonction de rappel pour gérer les données de la réponse contenant le code
    curl_easy_setopt(handle->curl, CURLOPT_WRITEFUNCTION, mycurl_generate_code);

    // Initialiser la variable qui stockera la chaîne de code
    receivedCode = NULL;

    // Passer l'adresse de receivedCode comme paramètre de rappel
    curl_easy_setopt(handle->curl, CURLOPT_WRITEDATA, &receivedCode);

    // Effectuer la requête
    CURLcode res = curl_easy_perform(handle->curl);

    // Vérifier les erreurs
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Erreur lors de la requête GET pour le code : %s\n", curl_easy_strerror(res));
        return NULL; // Indique une erreur
    }

    return receivedCode; // Succès
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