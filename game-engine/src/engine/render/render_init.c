#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "../util.h"
#include "../global.h"

#include "../render.h"
#include "../render_internal.h"

// Créer fenêtre + charger OPEN GL
SDL_Window *render_init_window(u32 width, u32 height)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        ERROR_EXIT("Initialisation SDL échouée", SDL_GetError());

    SDL_Window *window = SDL_CreateWindow(
        "MyGame",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        global.render.width,
        global.render.height,
        SDL_WINDOW_OPENGL);

    if (window == NULL)
        ERROR_EXIT("Création de fenêtre échouée", SDL_GetError());

    puts("Fenêtre creer");

    SDL_GL_CreateContext(window);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        ERROR_EXIT("Chargement de GL échoué", SDL_GetError());

    printf("OpenGL chargé !");

    printf("Fabriquant de la CG : %s\n", glGetString(GL_VENDOR));
    printf("Carte Graphique utilisée : %s\n", glGetString(GL_RENDERER));
    printf("Version OPEN GL : %s\n", glGetString(GL_VERSION));

    return window;
}

void render_init_shaders(Render_State_Internal *state)
{
    // Créer le shader par défaut             //Shader vertex (pour la forme)        //Shader frag (pour les couleurs)
    state->shader_default = render_shader_create("./shaders/default.vert", "./shaders/default.frag");

    /* [0001    Va créer une matrice 4x4
        0001    qui permet de representer
        0010    l'objet créé dans la fenêtre
        0100]   en 2D .                      */
    mat4x4_ortho(state->projection, 0, global.render.width, 0, global.render.height, -2, 2);

    // Lui appliquer le shader par défaut
    glUseProgram(state->shader_default);

    //  Envoie la matrice au shader dans le programme GL
    glUniformMatrix4fv(
        glGetUniformLocation(state->shader_default, "projection"),
        1,
        GL_FALSE,
        &state->projection[0][0]);
}

void render_init_color_texture(u32 *texture)
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    u8 solid_white[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, solid_white);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void render_init_quad(u32 *vao, u32 *vbo, u32 *ebo)
{
    // Les vertex (ou sommets) du quadrilataire.
    // (x,y,z) représentent les coordonnées
    // (u,v) représentent les coordonnées de textures = (0,1) -> Coin inférieur droit , (0,0) -> Coin superieur gauche
    //    x    y   z   u  v
    float vertices[] = {
        0.5, 0.5, 0, 0, 0,
        0.5, -0.5, 0, 0, 1,
        -0.5, -0.5, 0, 1, 1,
        -0.5, 0.5, 0, 1, 0};

    // Comme un quadrilataire est composé de deux triangles,
    // on définit l'ordre des indices de vertices des deux triangles qui vont composer notre quad,
    u32 indices[] = {
        0, 1, 3,  // Triangle 1 : Sommets (vertices[0], vertices[1], vertices[3])
        1, 2, 3}; // Triangle 2 : Sommets (vertices[1], vertices[2,, vertices[3])

    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glGenBuffers(1, ebo);

    glBindVertexArray(*vao);

    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // xyz
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glad_glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void render_init_line(u32 *vao, u32 *vbo)
{
    glGenVertexArrays(1, vao); // Generer un unique vao et le stocker dans la variable pointé par vao
    glBindVertexArray(*vao);   // Dire à OpenGL qu'on va travailler sur ces sommets là

    glGenBuffers(1, vbo);                // Generer un unique VBO et le stocker dans la variable pointé par vbo
    glBindBuffer(GL_ARRAY_BUFFER, *vbo); // Lier le vbo créé au gl_array_buffer  de openGL, pour dire qu'on va travailler avec

    // Allouer la place nécessaire pour les vbo et vao, on a besoin de le faire qu'au début
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);

    // On active l'index 0 du vbo pour le rendu
    glEnableVertexAttribArray(0);

    // On détache le vbo du gl_array_buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // On détache le vao de l'état d'openGL, pour dire qu'on a finit de travailler avec
    glBindVertexArray(0);
}