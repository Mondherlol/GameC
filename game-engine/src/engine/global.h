#pragma once

#include "render.h"
#include "config.h"
#include "input.h"
#include "time.h"
#include <stdbool.h>

typedef struct global
{
    Config_State config; //  Config des touches
    Input_State input;   // L'etat des touches
    Time_State time;
    float window_width;
    float window_height;
    bool should_quit;
} Global;

extern Global global;

extern bool game_started;