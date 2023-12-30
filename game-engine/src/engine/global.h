#pragma once

#include <stdbool.h>

#include "socket_server.h"
#include "render.h"
#include "config.h"
#include "input.h"
#include "time.h"
#include "types.h"
#include "my_curl.h"
#include "visitors.h"
#include "array_list.h"
#include "entity.h"

typedef enum screens
{
    GAME_SCREEN = 0,
    MENU_SCREEN = 1,
    SCORE_SCREEN = 2,
    GAME_OVER_SCREEN = 3,
    USERNAME_MENU_SCREEN = 4
} Screens;

typedef struct global
{
    Config_State config; //  Config des touches
    Input_State input;   // L'etat des touches
    Time_State time;
    float window_width;
    float window_height;
    bool should_quit;
    u8 current_screen;
    char generated_code[8]; // Code de la partie
    char username[6];
    MyCurlHandle curl_handle;
    MyCurlHandle post_curl_handle;
    SocketServer *server;
    Array_List *visitors; // Liste dynamique de visiteurs

} Global;

void spawn_enemy(Entity_Type enemy_type, bool is_enraged, bool is_flipped, Visitor *owner);

extern Global global;