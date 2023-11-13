#ifndef RENDER_INTERNAL_H
#define RENDER_INTERNAL_H

#include <SDL2/SDL.h>

#include "../types.h"
#include "render.h"

typedef struct render_state_interval
{
    u32 vao_quad; // Vertex Array Object pour dessiner un quad  - > OU Tableau d'array
    u32 vbo_quad; // Vectex Buffer Object pour dessiner un quad
    u32 ebo_quad; // Element Buffer Object pour dessiner un quad
    u32 vao_line;
    u32 vbo_line;
    u32 shader_default; // ID du shader par défaut
    u32 texture_color;  // ID de la couleur de texture par défaut
    mat4x4 projection;  // Matrice de projection
} Render_State_Internal;

SDL_Window *render_init_window(u32 width, u32 height);  // Initialiser la fenêtre
void render_init_quad(u32 *vao, u32 *vbo, u32 *ebo);    // Initialiser un quad pour le rendu
void render_init_color_texture(u32 *texture);           // Initialiser la texture de couleur
void render_init_shaders(Render_State_Internal *state); // Initialiser les shaders
void render_init_line(u32 *vao, u32 *vbo);
u32 render_shader_create(const char *path_vert, const char *path_frag); // Créer le shader en chargeant les fichiers Vertex et Fragment

#endif