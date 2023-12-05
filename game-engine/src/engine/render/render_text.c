#include <glad/glad.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "../render.h"
#include "../util.h"

static FT_Face face;
static FT_GlyphSlot g;

u32 text_vao;
u32 text_vbo;
u32 text_shader;
u32 text_texture;
mat4x4 projection;

typedef struct character_data
{
    u32 texture;
    u32 advance_x;
    u32 advance_y;
    i32 width;
    i32 height;
    i32 left;
    i32 top;
} Character_Data;

static Character_Data character_data_array[128];

void render_text_init()
{
    // Initialiser les shader pour le texte
    text_shader = render_shader_create("./shaders/text.vert", "./shaders/text.frag");

    glGenVertexArrays(1, &text_vao);
    glGenBuffers(1, &text_vbo);

    glBindVertexArray(text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Initialiser freetype
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        ERROR_EXIT("\nImpossibile d'initialiser Freetype \n");
    }

    // Charger font
    if (FT_New_Face(ft, "./assets/8-BIT_WONDER.TTF", 0, &face))
    {
        ERROR_EXIT("\nImpossible de charger la police d'ecriture \n");
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    g = face->glyph;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Parcourir tous les caractères
    for (u32 i = 0, n = 0; i < 128; ++i)
    {
        // Charger et l'associer à un glyph
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
        {
            printf("Impossible de lier le glyph '%c'\n", i);
            continue;
        }

        // Initialiser sa texture
        u32 texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            g->bitmap.width,
            g->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            g->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        Character_Data *character_data = &character_data_array[n++];
        character_data->texture = texture;
        character_data->advance_x = g->advance.x;
        character_data->advance_y = g->advance.y;
        character_data->width = g->bitmap.width;
        character_data->height = g->bitmap.rows;
        character_data->top = g->bitmap_top;
        character_data->left = g->bitmap_left;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &text_vao);
    glGenBuffers(1, &text_vbo);
    glBindVertexArray(text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Initialiser la matrice de project pour chaque shader
    mat4x4_ortho(projection, 0, render_width, 0, render_height, -2.0f, 2.0f);

    mat4x4 text_projection;
    mat4x4_identity(text_projection);
    mat4x4_ortho(text_projection, 0, render_width * scale, 0, render_height * scale, -2.0f, 2.0f);
    glUseProgram(text_shader);
    glUniformMatrix4fv(glGetUniformLocation(text_shader, "projection"), 1, GL_FALSE, &text_projection[0][0]);
}

void render_text(const char *text, float x, float y, vec4 color, u8 is_centered)
{
    glUseProgram(text_shader);

    glUniform4fv(glGetUniformLocation(text_shader, "color"), 1, color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(text_vao);

    x *= 2;
    y *= 2;

    if (is_centered)
    {
        float width = 0;

        for (const char *p = text; *p; ++p)
        {
            width += character_data_array[(u32)*p].width;
        }

        x -= width * 0.5;
    }

    for (const char *p = text; *p; ++p)
    {
        Character_Data cd = character_data_array[(u32)*p];

        float x2 = x + cd.left;
        float y2 = y - (cd.height - cd.top);
        float w = cd.width;
        float h = cd.height;

        float vertices[6][4] = {
            {x2, y2 + h, 0, 0},
            {x2, y2, 0, 1},
            {x2 + w, y2, 1, 1},

            {x2, y2 + h, 0, 0},
            {x2 + w, y2, 1, 1},
            {x2 + w, y2 + h, 1, 0}};

        glBindTexture(GL_TEXTURE_2D, cd.texture);
        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (cd.advance_x / 64);
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}