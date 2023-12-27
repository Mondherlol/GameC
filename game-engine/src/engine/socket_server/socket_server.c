#include <stdio.h>

#include "../socket_server.h"
#include "../global.h"
#include "../util.h"

static void send_create_game_message()
{
    char message[256];
    snprintf(message, sizeof(message), "create-game | %s", global.username);
    send(global.server->client_socket, message, strlen(message), 0);
}

static void send_close_connexion()
{
    char message[256];
    snprintf(message, sizeof(message), "close");
    send(global.server->client_socket, message, strlen(message), 0);
}

void server_init()
{
    global.server = NULL;
    SocketServer *server = malloc(sizeof(SocketServer));
    if (server == NULL)
        ERROR_EXIT("Erreur d'allocation de mémoire pour le serveur.\n");

    WSADATA wsa;
    // Initialiser Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        free(server);
        ERROR_EXIT("Échec de l'initialisation de Winsock\n");
    }

    // Créer le socket du serveur
    server->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_socket == INVALID_SOCKET)
    {
        server_cleanup(server);
        ERROR_EXIT("Erreur lors de la création du socket : %d\n", WSAGetLastError());
    }

    // Initialiser les informations du serveur
    memset(&server->server_addr, 0, sizeof(server->server_addr));
    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = INADDR_ANY;
    server->server_addr.sin_port = htons(PORT);

    // Initialiser la longueur de l'adresse du client
    server->client_addr_len = sizeof(server->client_addr);

    // Lier le socket à l'adresse et au port
    if (bind(server->server_socket, (struct sockaddr *)&server->server_addr, sizeof(server->server_addr)) == SOCKET_ERROR)
    {
        server_cleanup(server);
        ERROR_EXIT("Erreur lors de la liaison du socket : %d\n", WSAGetLastError());
    }

    // Mettre le socket en mode écoute
    if (listen(server->server_socket, 5) == SOCKET_ERROR)
    {
        server_cleanup(server);
        ERROR_EXIT("Erreur lors de la mise en écoute du socket : %d\n", WSAGetLastError());
    }

    printf("En attente de connexion...\n");

    // Accepter la connexion du client
    server->client_socket = accept(server->server_socket, (struct sockaddr *)&server->client_addr, &server->client_addr_len);
    if (server->client_socket == INVALID_SOCKET)
    {
        printf("Erreur lors de l'acceptation de la connexion : %d\n", WSAGetLastError());
        server_cleanup(server);
        global.server = NULL;
        return;
    }

    printf("\nConnexion établie avec le client.\n");
    global.server = server;

    printf("Mise en écoute....\n");

    send_create_game_message();

    receive_data();
}

void server_cleanup()
{
    if (global.server != NULL)
    {
        send_close_connexion();
        // Fermer les sockets et désinitialiser Winsock
        closesocket(global.server->client_socket);
        closesocket(global.server->server_socket);
        WSACleanup();
        free(global.server);
        global.server = NULL;
        printf("\nSocket fermer et winsock desinitialiser.");
    }
}

void receive_data()
{
    while (global.server != NULL)
    {

        // Recevoir des données du client
        int bytes_received = recv(global.server->client_socket, global.server->buffer, sizeof(global.server->buffer), 0);
        if (bytes_received == SOCKET_ERROR)
        {
            printf("Erreur lors de la réception de données : %d\n", WSAGetLastError());
        }
        else if (bytes_received > 0)
        {
            // Traiter les données reçues
            global.server->buffer[bytes_received] = '\0'; // Assurer la null-termination pour l'affichage
            printf("Données reçues du client : %s\n", global.server->buffer);
            // Copier les données reçues dans la variable message
            if (strcmp(global.server->buffer, "Deconnexion") == 0)
            {
                global.server = NULL;
                printf("Le serveur python s'est fermer.");
            }
            else if (strncmp(global.server->buffer, "code=", strlen("code=")) == 0)
            {
                char *code = global.server->buffer + strlen("code=");
                printf("Code extrait : %s\n", code);
                strcpy(global.generated_code, code);
            }
        }
    }
}