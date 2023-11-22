#include <glad/glad.h>

#include "../global.h"
#include "../render.h"
#include "../render_internal.h"

static Render_State_Internal state = {0};

void render_init(void)
{
    global.render.width = 1080;
    global.render.height = 720;
    global.render.window = render_init_window(global.render.width, global.render.height);

    render_init_quad(&state.vao_quad, &state.vbo_quad, &state.ebo_quad); // Initier le rendu du quad
    render_init_line(&state.vao_line, &state.vbo_line);
    render_init_shaders(&state);
    render_init_color_texture(&state.texture_color);

    glEnable(GL_BLEND);                                // Active le mélange de couleur d'Open GL
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Indique une méthode de mélange de couleur en gros
}
void render_begin(void)
{
    glClearColor(0.08, 0.1, 0.1, 1); // Couleur d'effacement de l'écran en début de rendu
    glClear(GL_COLOR_BUFFER_BIT);    // Effacer tampon de couleur
}
void render_end(void)
{
    SDL_GL_SwapWindow(global.render.window); // Mettre à jour la fenêtre avec le rendu OPENGL
}
void render_quad(vec2 pos, vec2 size, vec4 color)
{
    glUseProgram(state.shader_default);

    mat4x4 model;
    mat4x4_identity(model);

    mat4x4_translate(model, pos[0], pos[1], 0);
    mat4x4_scale_aniso(model, model, size[0], size[1], 1);

    glUniformMatrix4fv(glGetUniformLocation(state.shader_default, "model"),
                       1,
                       GL_FALSE,
                       &model[0][0]);

    glUniform4fv(glad_glGetUniformLocation(state.shader_default, "color"),
                 1,
                 color);

    glBindVertexArray(state.vao_quad);

    glBindTexture(GL_TEXTURE_2D, state.texture_color);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
}

void render_line_segment(vec2 start, vec2 end, vec4 color)
{
    glUseProgram(state.shader_default); // Utiliser le shader par défaut
    glLineWidth(3);

    float x = end[0] - start[0]; // x final - x de départ -> Ca donne la longueur x du trait
    float y = end[1] - start[1]; // y final - y de départ -> De combien doit -il monter ?

    //              [x  y  z  x  y  z] pour le dessiner ,
    float line[6] = {0, 0, 0, x, y, 0};

    mat4x4 model; // Initialiser matrice (elle sert à poser en quelque sorte le point de départ du dessin)

    // Matrice                 x          y     z
    mat4x4_translate(model, start[0], start[1], 0); // On la place sur le point de depart

    // On transmet la matrice de transformation (donc avec la position de depart) au shader par défaut
    glUniformMatrix4fv(glGetUniformLocation(state.shader_default, "model"),
                       1,
                       GL_FALSE,
                       &model[0][0]);

    // On transmet la couleur au shader
    glUniform4fv(glGetUniformLocation(state.shader_default, "color"),
                 1,
                 color);
    glBindTexture(GL_TEXTURE_2D, state.texture_color); // On dit à OpenGL qu'on va utiliser cette couleur
    glBindVertexArray(state.vao_line);                 // On dit à OpenGL qu'on va dessiner sur ces points

    glBindBuffer(GL_ARRAY_BUFFER, state.vbo_line); // Pareil avec le vbo
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line), line);

    glDrawArrays(GL_LINES, 0, 2); // Dessiner la ligne

    glBindVertexArray(0); // Retirer le vao_line de l'api OpenGL car c'est bon on a finit avec
}

void render_quad_line(vec2 pos, vec2 size, vec4 color)
{
    // On stocke dans un tableau les VEC des 4 points composant le quad
    vec2 points[4] = {
        {pos[0] - size[0] * 0.5, pos[1] - size[1] * 0.5}, // Point haut gauche du quad
        {pos[0] + size[0] * 0.5, pos[1] - size[1] * 0.5}, // Point Haut droite
        {pos[0] + size[0] * 0.5, pos[1] + size[1] * 0.5}, // Point Bas Droite
        {pos[0] - size[0] * 0.5, pos[1] + size[1] * 0.5}, // Point bas gauche
    };

    // Et mnt on relie les points TADAA ça fait un quad
    render_line_segment(points[0], points[1], color);
    render_line_segment(points[1], points[2], color);
    render_line_segment(points[2], points[3], color);
    render_line_segment(points[3], points[0], color);
}

void render_aabb(float *aabb, vec4 color)
{
    vec2 size;

    vec2_scale(size, &aabb[2], 2); // On multiplie le size du aabb par 2
                                   // En gros on l'aggrandit car on a définit la taille d'un aabb par une halfsize dans leur structure

    render_quad_line(&aabb[0], size, color);
}