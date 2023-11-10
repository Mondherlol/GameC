#pragma once

#include "types.h"

typedef struct time_state
{
    float delta; // Duree entre deux frames
    float now;   // Temps de la frame actuelle
    float last;  // Temps de la frame précedente

    float frame_last;  // Derniere fois que le calcul de fps a eu lieu
    float frame_delay; // Duree MAX d'une frame
    float frame_time;  // Combien de temps la frame est restée affichée

    u32 frame_rate;  // FPS
    u32 frame_count; // Aide a calculer le frame rate

} Time_State;

void time_init(u32 frame_rate);

// Appelée à chaque frame
void time_update(void);

// Appelée à la fin de chaque frame
void time_update_late(void);
