#pragma once

#include "render.h"
#include "config.h"
#include "input.h"
#include "time.h"

typedef struct global
{
    Config_State config; //  Config des touches
    Input_State input;   // L'etat des touches
    Time_State time;
} Global;

extern Global global;
