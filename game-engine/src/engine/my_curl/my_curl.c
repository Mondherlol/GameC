
#include "../my_curl.h"

int mycurl_init(MyCurlHandle *handle)
{
    // Initialiser libcurl
    handle->curl = curl_easy_init();

    if (!handle->curl)
    {
        fprintf(stderr, "Erreur lors de l'initialisation de libcurl\n");
        return 1; // Indique une erreur
    }

    return 0; // Succès
}

#include <stdlib.h> // Pour malloc et free

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