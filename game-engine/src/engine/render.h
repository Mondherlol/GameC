#pragma once

#include <SDL2/SDL.h>
#include <linmath.h> //Librairie qui sert à avoir des vecteurs (vec2, vec4,...)

typedef struct render_state // L'état de la fenêtre de rendu
{
    SDL_Window *window;
    float width;
    float height;
} Render_State;

// Procédures habituelles de rendus
void render_init(void);
void render_begin(void);
void render_end(void);
void render_quad(vec2 pos, vec2 size, vec4 color);
void render_line_segment(vec2 start, vec2 end, vec4 color);
void render_quad_line(vec2 pos, vec2 size, vec4 color);
void render_aabb(float *aabb, vec4 color);