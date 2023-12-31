#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#include "../socket_server.h"
#include "../global.h"
#include "../util.h"
#include "../visitors.h"

static void start_python_server()
{

    const char *executablePath = "start /B socket_server.exe";

    // Utilisez la fonction system pour lancer le programme externe en arrière-plan
    int result = system(executablePath);

    // Vérifiez le résultat pour détecter d'éventuelles erreurs
    if (result == 0)
    {
        printf("Le programme externe a été lancé avec succès.\n");
    }
    else
    {
        printf("Erreur lors du lancement du programme externe.\n");
    }
}

static void send_create_game_message()
{
    char message[50];
    snprintf(message, sizeof(message), "create-game | %s", global.username);
    send(global.server->client_socket, message, strlen(message), 0);
}

void send_game_statut(bool isGameInProgress, char killed_by[30])
{
    char message[50];
    snprintf(message, sizeof(message), "game-statut | %s | %s", isGameInProgress ? "true" : "false", killed_by);
    send(global.server->client_socket, message, strlen(message), 0);
}

static void send_close_connexion()
{
    char message[10];
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

    // Executer le serveur python
    start_python_server();

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

// Retirer les préfixes pour les données
static void removePrefix(char *str, const char *prefix)
{
    size_t prefixLength = strlen(prefix);
    char *ptr = strstr(str, prefix);
    if (ptr != NULL)
    {
        // Déplacer le pointeur au-delà du préfixe
        memmove(ptr, ptr + prefixLength, strlen(ptr + prefixLength) + 1);
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
            int error_code = WSAGetLastError();
            if (error_code == WSAECONNRESET || error_code == WSAENETRESET)
            {
                // Le client s'est déconnecté proprement, arrêter la boucle
                printf("Le client s'est déconnecté proprement.\n");
                break;
            }
            else
            {
                // Une autre erreur s'est produite
                printf("Erreur lors de la réception de données : %d\n", error_code);
                printf("Vérifiez que vous êtes bien connectés à Internet.");
                break;
            }
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
            else if (strncmp(global.server->buffer, "new_visitor=", strlen("new_visitor=")) == 0)
            {
                char *new_visitor = global.server->buffer + strlen("new_visitor=");
                char *username = strtok(new_visitor, "&");
                char *socketId = strtok(NULL, "&");
                // Retirer le préfixe "socketId="
                removePrefix(socketId, "socketId=");

                printf("Nouveau visiteur : %s avec id %s\n", username, socketId);
                addVisitor(username, socketId);
            }
            else if (strncmp(global.server->buffer, "leaving_visitor=", strlen("leaving_visitor=")) == 0)
            {
                char *leaving_visitor_id = global.server->buffer + strlen("leaving_visitor=");
                printf("Visiteur avec id %s  a quitter la partie \n", leaving_visitor_id);
                removeVisitor(leaving_visitor_id);
            }
            else if (strncmp(global.server->buffer, "new_enemy=", strlen("new_enemy=")) == 0)
            {

                char *new_enemy = global.server->buffer + strlen("new_enemy=");
                char *enemy_type_str = strtok(new_enemy, "&");
                char *socketId = strtok(NULL, "&");
                removePrefix(socketId, "socketId=");

                // Convertir la chaîne en nombre
                int enemy_type = atoi(enemy_type_str);
                // Recuperer le visiteur
                Visitor *owner = getVisitor(socketId);

                bool is_flipped = rand() % 100 >= 50;

                spawn_enemy(enemy_type, true, is_flipped, owner);
                printf("Nouvel ennemi : %d envoyer par  %s  a ete spawn\n", enemy_type, socketId);
            }
        }
    }
}