#pragma once

#include "../array_list.h"
#include "../types.h"

// Contient un pointeur vers une liste de body auxquels on va manipuler la physique
typedef struct physics_state_internal
{
    Array_List *body_list;
} Physics_State_Internal;
