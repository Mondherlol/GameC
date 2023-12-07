#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION // D'apres le readme du github de STB
#include <stb_image.h>

#include "../global.h"
#include "../render.h"
#include "../render_internal.h"
#include "../array_list.h"
#include "../util.h"

static u32 vao_quad;       // Vertex Array Object pour dessiner un quad  - > OU Tableau d'array
static u32 vbo_quad;       // Vectex Buffer Object pour dessiner un quad
static u32 ebo_quad;       // Element Buffer Object pour dessiner un quad
static u32 vao_line;       // Vao pour dessiner une ligne
static u32 vbo_line;       // Vbo pour dessiner la ligne
static u32 shader_default; // ID du shader par défaut
static u32 texture_color;  // ID de la couleur de texture par défaut
static u32 vao_batch;
static u32 vbo_batch;
static u32 ebo_batch;
static u32 shader_batch;

static Array_List *list_batch;

SDL_Window *render_init(void)
{
    SDL_Window *window = render_init_window(window_width, window_height);
    global.window_height = window_height;
    global.window_width = window_width;

    render_init_quad(&vao_quad, &vbo_quad, &ebo_quad); // Initier le rendu du quad
    render_init_batch_quads(&vao_batch, &vbo_batch, &ebo_batch);
    render_init_line(&vao_line, &vbo_line);
    render_init_shaders(&shader_default, &shader_batch, render_width, render_height);
    render_init_color_texture(&texture_color);

    glEnable(GL_BLEND);                                // Active le mélange de couleur d'Open GL
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Indique une méthode de mélange de couleur en gros

    list_batch = array_list_create(sizeof(Batch_Vertex), 8);

    stbi_set_flip_vertically_on_load(1); // Retourner les images au chargement car sinon elles sont inversées

    return window;
}

void render_begin(void)
{
    glClearColor(0.08, 0.1, 0.1, 1); // Couleur d'effacement de l'écran en début de rendu
    glClear(GL_COLOR_BUFFER_BIT);    // Effacer tampon de couleur

    list_batch->len = 0;
}

static void render_batch(Batch_Vertex *vertices, size_t count, u32 texture_ids[8])
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo_batch);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(Batch_Vertex), vertices);

    // Texture de couleur
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_ids[1]);

    // Le reste des textures
    for (u32 i = 1; i < 8; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, texture_ids[i]);
    }

    glUseProgram(shader_batch);
    glBindVertexArray(vao_batch);

    glDrawElements(GL_TRIANGLES, (count >> 2) * 6, GL_UNSIGNED_INT, NULL);
}

static void append_quad(vec2 position, vec2 size, vec4 texture_coordinates, vec4 color, float texture_slot)
{
    vec4 uvs = {0, 0, 1, 1};

    if (texture_coordinates != NULL)
    {
        memcpy(uvs, texture_coordinates, sizeof(vec4));
    }

    array_list_append(list_batch, &(Batch_Vertex){
                                      .position = {position[0], position[1]},
                                      .uvs = {uvs[0], uvs[1]},
                                      .color = {color[0], color[1], color[2], color[3]},
                                      .texture_slot = texture_slot});

    array_list_append(list_batch, &(Batch_Vertex){
                                      .position = {position[0] + size[0], position[1]},
                                      .uvs = {uvs[2], uvs[1]},
                                      .color = {color[0], color[1], color[2], color[3]},
                                      .texture_slot = texture_slot

                                  });

    array_list_append(list_batch, &(Batch_Vertex){
                                      .position = {position[0] + size[0], position[1] + size[1]},
                                      .uvs = {uvs[2], uvs[3]},
                                      .color = {color[0], color[1], color[2], color[3]},
                                      .texture_slot = texture_slot});

    array_list_append(list_batch, &(Batch_Vertex){
                                      .position = {position[0], position[1] + size[1]},
                                      .uvs = {uvs[0], uvs[3]},
                                      .color = {color[0], color[1], color[2], color[3]},
                                      .texture_slot = texture_slot

                                  });
}

void render_end(SDL_Window *window, u32 batch_texture_ids[8])
{
    render_batch(list_batch->items, list_batch->len, batch_texture_ids);

    SDL_GL_SwapWindow(window); // Mettre à jour la fenêtre avec le rendu OPENGL
}

void render_quad(vec2 pos, vec2 size, vec4 color)
{
    glUseProgram(shader_default);

    mat4x4 model;
    mat4x4_identity(model);

    mat4x4_translate(model, pos[0], pos[1], 0);
    mat4x4_scale_aniso(model, model, size[0], size[1], 1);

    glUniformMatrix4fv(glGetUniformLocation(shader_default, "model"),
                       1,
                       GL_FALSE,
                       &model[0][0]);

    glUniform4fv(glad_glGetUniformLocation(shader_default, "color"),
                 1,
                 color);

    glBindVertexArray(vao_quad);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_color);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
}

void render_line_segment(vec2 start, vec2 end, vec4 color)
{
    glUseProgram(shader_default); // Utiliser le shader par défaut
    glLineWidth(3);

    float x = end[0] - start[0]; // x final - x de départ -> Ca donne la longueur x du trait
    float y = end[1] - start[1]; // y final - y de départ -> De combien doit -il monter ?

    //              [x  y  z  x  y  z] pour le dessiner ,
    float line[6] = {0, 0, 0, x, y, 0};

    mat4x4 model; // Initialiser matrice (elle sert à poser en quelque sorte le point de départ du dessin)

    // Matrice                 x          y     z
    mat4x4_translate(model, start[0], start[1], 0); // On la place sur le point de depart

    // On transmet la matrice de transformation (donc avec la position de depart) au shader par défaut
    glUniformMatrix4fv(glGetUniformLocation(shader_default, "model"),
                       1,
                       GL_FALSE,
                       &model[0][0]);

    // On transmet la couleur au shader
    glUniform4fv(glGetUniformLocation(shader_default, "color"),
                 1,
                 color);
    glBindTexture(GL_TEXTURE_2D, texture_color); // On dit à OpenGL qu'on va utiliser cette couleur
    glBindVertexArray(vao_line);                 // On dit à OpenGL qu'on va dessiner sur ces points

    glBindBuffer(GL_ARRAY_BUFFER, vbo_line); // Pareil avec le vbo
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

static u32 next_sprite_sheet_slot = 1;

void render_sprite_sheet_init(Sprite_Sheet *sprite_sheet, const char *path, float cell_width, float cell_height)
{
    glGenTextures(1, &sprite_sheet->texture_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sprite_sheet->texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, channel_count;
    u8 *image_data = stbi_load(path, &width, &height, &channel_count, 0);
    if (!image_data)
    {
        ERROR_EXIT("Erreur lors du chargement de l'image :  %s\n", path);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    sprite_sheet->width = (float)width;
    sprite_sheet->height = (float)height;
    sprite_sheet->cell_width = cell_width;
    sprite_sheet->cell_height = cell_height;
    sprite_sheet->texture_slot = next_sprite_sheet_slot++;
}

static void calculate_sprite_texture_coordinates(vec4 result, float row, float column, float texture_width, float texture_height, float cell_width, float cell_height)
{
    float w = 1.0 / (texture_width / cell_width);
    float h = 1.0 / (texture_height / cell_height);
    float x = column * w;
    float y = row * h;
    result[0] = x;
    result[1] = y;
    result[2] = x + w;
    result[3] = y + h;
}

static i32 find_texture_slot(u32 texture_slots[8], u32 texture_id)
{
    for (i32 i = 1; i < 8; ++i)
    {
        if (texture_slots[i] == texture_id)
        {
            return i;
        }
    }
    return -1;
}

static i32 try_insert_texture(u32 texture_slots[8], u32 texture_id)
{
    i32 index = find_texture_slot(texture_slots, texture_id);

    if (index > 0)
        return index;

    for (i32 i = 1; i < 8; ++i)
    {
        if (texture_slots[i] == 0)
        {
            texture_slots[i] = texture_id;
            return i;
        }
    }
    return -1;
}

void render_sprite_sheet_frame(Sprite_Sheet *sprite_sheet, float row, float column, vec2 position, bool is_flipped, vec4 color, float texture_slot)
{
    vec4 uvs;
    calculate_sprite_texture_coordinates(uvs, row, column, sprite_sheet->width, sprite_sheet->height, sprite_sheet->cell_width, sprite_sheet->cell_height);

    if (is_flipped)
    {
        float tmp = uvs[0];
        uvs[0] = uvs[2];
        uvs[2] = tmp;
    }

    vec2 size = {sprite_sheet->cell_width, sprite_sheet->cell_height};
    vec2 bottom_left = {position[0] - size[0] * 0.5, position[1] - size[1] * 0.5};

    append_quad(bottom_left, size, uvs, color, texture_slot);
}

float render_get_scale()
{
    return scale;
}

// Initialisation d'une image
void init_image(Image *image, const char *path)
{
    // Chargement de l'image
    int image_width, image_height, channel_count;
    stbi_set_flip_vertically_on_load(1);
    u8 *image_data = stbi_load(path, &image_width, &image_height, &channel_count, 0);
    if (!image_data)
    {
        ERROR_EXIT("Erreur lors du chargement de l'image :  %s\n", path);
    }

    // Initialisation de la texture
    glGenTextures(1, &image->texture_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image->texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLenum format = (channel_count == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    // stbi_image_free(image_data);

    image->width = (float)image_width;
    image->height = (float)image_height;
}

// Ajout d'une image à la liste de lots
void render_image(Image *image, vec2 position, vec2 size, u32 texture_slots[8])
{
    vec4 texture_coordinates = {0.0f, 0.0f, 1.0f, 1.0f};

    i32 texture_slot = try_insert_texture(texture_slots, image->texture_id);
    if (texture_slot == -1)
    {
        // Faire autre chose
    }

    append_quad(position, size, texture_coordinates, (vec4){1, 1, 1, 1}, (float)texture_slot);
}
