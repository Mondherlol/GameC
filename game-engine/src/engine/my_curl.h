#pragma once

#include <stdio.h>
#include <curl/curl.h>
#include <windows.h>

// Définition de la constante pour l'adresse du serveur
#define SERVER_URL "http://localhost:3001"

// Déclaration d'une structure pour stocker les informations nécessaires à libcurl
typedef struct myCurlHandle
{
    CURL *curl;
} MyCurlHandle;

typedef struct curlRequestData
{
    MyCurlHandle *handle;
    const char *endpoint;
} CurlRequestData;

typedef void (*AsyncCallback)(const char *);

typedef struct
{
    MyCurlHandle *handle;
    const char *endpoint;
    const char *post_data;
    AsyncCallback callback_function;
} CurlPostRequestData;

// Fonction pour initialiser libcurl
void mycurl_init(MyCurlHandle *handle);

// Fonction pour effectuer une requête GET
int mycurl_get(MyCurlHandle *handle, const char *endpoint);

// Fonction pour effectuer une requête POST avec des données
int mycurl_post(MyCurlHandle *handle, const char *endpoint, const char *post_data);

// Fonction pour libérer les ressources
void mycurl_cleanup(MyCurlHandle *handle);

// Faire une requête GET dans un thread séparé
DWORD WINAPI async_curl_request(LPVOID data);

int mycurl_post_async(MyCurlHandle *handle, const char *endpoint, const char *post_data, AsyncCallback callback_function);
DWORD WINAPI async_curl_post_request(LPVOID data);