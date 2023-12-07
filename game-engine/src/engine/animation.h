#pragma once

#include "render.h"

#define MAX_FRAMES 16 // Frame à afficher par seconde de l'animation

typedef struct animation_frame
{
    float duration; // Combien d'images
    u8 row;         // Quelle ligne
    u8 column;      // Quelle colonne
} Animation_Frame;

typedef struct animation_definition
{
    Sprite_Sheet *sprite_sheet;
    Animation_Frame frames[MAX_FRAMES];
    u8 frame_count;
} Animation_Definition;

typedef struct animation
{
    Animation_Definition *definition;
    float current_frame_time;
    u8 current_frame_index;
    bool does_loop;
    bool is_active;
    bool is_flipped;
} Animation;

void animation_init(void);
size_t animation_definition_create(Sprite_Sheet *sprite_sheet, float *durations, u8 *rows, u8 *columns, u8 frame_count);
size_t animation_create(size_t animation_definition_id, bool does_loop);
void animation_destroy(size_t id);
Animation *animation_get(size_t id);
void animation_update(float dt);
void animation_render(Animation *animation, vec2 position, vec4 color);