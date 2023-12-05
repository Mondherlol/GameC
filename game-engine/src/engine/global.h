#pragma once

#include "render.h"
#include "config.h"
#include "input.h"
#include "time.h"
#include <stdbool.h>
#include "types.h"

typedef enum screens
{
    GAME_SCREEN = 0,
    MENU_SCREEN = 1,
    SETTINGS_SCREEN = 2
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

} Global;

extern Global global;