#pragma once

#include "input.h"
#include "types.h"

// La configuration des touches du joueur
typedef struct config
{
    u8 keybinds[6];
} Config_State;

void config_init(void);
void config_key_bind(Input_Key key, const char *key_name);
void save_username(char *new_username);