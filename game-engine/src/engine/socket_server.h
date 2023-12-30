#ifndef SIMPLE_SERVER_H
#define SIMPLE_SERVER_H

#include <winsock.h>
#include <stdbool.h>

// #include <winsock2.h>

#define PORT 12345

typedef struct
{
    SOCKET server_socket;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int client_addr_len;
    char buffer[1024];
    char message[1024];
} SocketServer;

void server_init();
void server_cleanup();
void receive_data();
void send_game_statut(bool isGameInProgress, char killed_by[30]);

#endif
