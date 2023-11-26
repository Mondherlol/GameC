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
        width,
        height,
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

void render_init_shaders(u32 *shader_default, u32 *shader_batch, float render_width, float render_height)
{
    mat4x4 projection; // Matrice de projection

    // Créer le shader par défaut       //Shader vertex (pour la forme)   //Shader frag (pour les couleurs)
    *shader_default = render_shader_create("./shaders/default.vert", "./shaders/default.frag");

    *shader_batch = render_shader_create("./shaders/batch_quad.vert", "./shaders/batch_quad.frag");

    /* [0001    Va créer une matrice 4x4
        0001    qui permet de representer
        0010    l'objet créé dans la fenêtre
        0100]   en 2D .                      */
    mat4x4_ortho(projection, 0, render_width, 0, render_height, -2, 2);

    // Lui appliquer le shader par défaut
    glUseProgram(*shader_default);

    //  Envoie la matrice au shader dans le programme GL
    glUniformMatrix4fv(
        glGetUniformLocation(*shader_default, "projection"),
        1,
        GL_FALSE,
        &projection[0][0]);

    // Pareil avec le shader batch
    glUseProgram(*shader_batch);

    glUniformMatrix4fv(
        glGetUniformLocation(*shader_batch, "projection"),
        1,
        GL_FALSE,
        &projection[0][0]);
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

void render_init_batch_quads(u32 *vao, u32 *vbo, u32 *ebo)
{
    glGenVertexArrays(1, vao); // Generer un tableau d'objets vertex
    glBindVertexArray(*vao);   // Lier le VAO

    u32 indices[MAX_BATCH_ELEMENTS]; // Créer un tableau d'indices pour les quadrilataires

    // Un quad est composé de deux triangles, donc 6 indices en gros
    for (u32 i = 0, offset = 0; i < MAX_BATCH_ELEMENTS; i += 6, offset += 4)
    {
        // Premier triangle
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;
        // Deuxieme triangle
        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;
    }

    glGenBuffers(1, vbo);                                                                            // Génerer un vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);                                                             // Le lier
    glBufferData(GL_ARRAY_BUFFER, MAX_BATCH_VERTICES * sizeof(Batch_Vertex), NULL, GL_DYNAMIC_DRAW); // Allouer l'espace pour les données

    // Spécifier la structure des données dans le VBO
    // [x, y]
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Batch_Vertex), (void *)offsetof(Batch_Vertex, position));
    // [u, v]
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Batch_Vertex), (void *)offsetof(Batch_Vertex, uvs));
    // [r, g, b, a]
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Batch_Vertex), (void *)offsetof(Batch_Vertex, color));

    glGenBuffers(1, ebo);                                                                             // Generer un element buffer object
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);                                                      // Le lier
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_BATCH_ELEMENTS * sizeof(u32), indices, GL_STATIC_DRAW); // Lui transmettre les indices

    glBindVertexArray(0);                     // Détacher vao
    glBindBuffer(GL_ARRAY_BUFFER, 0);         // Détacher vbo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Detacher ebo
}