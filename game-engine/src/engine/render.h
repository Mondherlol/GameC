#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <linmath.h> //Librairie qui sert à avoir des vecteurs (vec2, vec4,...)

typedef struct render_state // L'état de la fenêtre de rendu
{
    SDL_Window *window;
    float width;
    float height;
} Render_State;

// Procédures habituelles de rendus pour éviter la redondance
void render_init(void);
void render_begin(void);
void render_end(void);
void render_quad(vec2 pos, vec2 size, vec4 color);

// #include "../types.h"

#endif