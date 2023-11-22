#ifndef RENDER_INTERNAL_H
#define RENDER_INTERNAL_H

#include <SDL2/SDL.h>

#include "../types.h"
#include "render.h"

SDL_Window *render_init_window(u32 width, u32 height);                                  // Initialiser la fenêtre
void render_init_quad(u32 *vao, u32 *vbo, u32 *ebo);                                    // Initialiser un quad pour le rendu
void render_init_color_texture(u32 *texture);                                           // Initialiser la texture de couleur
void render_init_shaders(u32 *shader_default, float render_width, float render_height); // Initialiser les shaders
void render_init_line(u32 *vao, u32 *vbo);                                              // Initier le rendu de la ligne
u32 render_shader_create(const char *path_vert, const char *path_frag);                 // Créer le shader en chargeant les fichiers Vertex et Fragment

#endif