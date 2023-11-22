#pragma once

#include <SDL2/SDL.h>
#include <linmath.h> //Librairie qui sert à avoir des vecteurs (vec2, vec4,...)

typedef struct batch_vertex
{
    vec2 position;
    vec2 uvs;
    vec4 color;
} Batch_Vertex;

#define MAX_BATCH_QUADS 10000
#define MAX_BATCH_VERTICES 40000
#define MAX_BATCH_ELEMENTS 60000

// Procédures habituelles de rendus
SDL_Window *render_init(void);
void render_begin(void);
void render_end(SDL_Window *window);
void render_quad(vec2 pos, vec2 size, vec4 color);
void render_line_segment(vec2 start, vec2 end, vec4 color); // Dessiner une ligne
void render_quad_line(vec2 pos, vec2 size, vec4 color);     // Dessiner les lignes d'un quad
void render_aabb(float *aabb, vec4 color);
float render_get_scale();

void append_quad(vec2 position, vec2 size, vec4 texture_coordinates, vec4 color);