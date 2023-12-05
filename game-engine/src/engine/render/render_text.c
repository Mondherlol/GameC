#include <ft2build.h>
#include FT_FREETYPE_H

#include "../render.h"
#include <glad/glad.h>

static FT_Face face;
static FT_GlyphSlot g;

static float WIDTH = 640;
static float HEIGHT = 360;
static float SCALE = 2;

typedef struct render_state
{
    u32 text_vao;
    u32 text_vbo;
    u32 text_shader;
    u32 text_texture;
    mat4x4 projection;

} Render_State;

Render_State render_state = {0};
static Render_State *state = &render_state;

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
char *io_file_read_test(const char *path);

static u32 shader_setup(const char *vert_path, const char *frag_path);

void error_and_exit(i32 code, const char *message)
{
    fprintf(stderr, "Error: %s\n", message);
    exit(code);
}

char *io_file_read_test(const char *path)
{
    FILE *fp = fopen(path, "r");

    if (!fp)
    {
        printf("Cannot read file %s\n", path);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);

    size_t length = ftell(fp);

    if (length == -1L)
    {
        printf("Could not assertain length of file %s\n", path);
        return NULL;
    }

    fseek(fp, 0, SEEK_SET);

    char *buffer = malloc((length + 1) * sizeof(char));
    if (!buffer)
    {
        printf("Cannot allocate file buffer for %s\n", path);
        return NULL;
    }

    fread(buffer, sizeof(char), length, fp);
    buffer[length] = 0;

    fclose(fp);

    printf("File loaded. %s\n", path);
    return buffer;
}

static u32 shader_setup(const char *vert_path, const char *frag_path)
{

    int success;
    char log[512];
    char *vertex_source = io_file_read_test(vert_path);

    uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const char *const *)&vertex_source, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, log);
        error_and_exit(-1, log);
    }

    char *fragment_source = io_file_read_test(frag_path);
    uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const char *const *)&fragment_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, log);
        error_and_exit(-1, log);
    }

    u32 shader = glCreateProgram();
    glAttachShader(shader, vertex_shader);
    glAttachShader(shader, fragment_shader);
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader, 512, NULL, log);
        error_and_exit(-1, log);
    }

    free(vertex_source);
    free(fragment_source);

    return shader;
}

void render_text_init()
{

    // Setup text shader.
    state->text_shader = shader_setup("./shaders/text.vert", "./shaders/text.frag");

    glGenVertexArrays(1, &state->text_vao);
    glGenBuffers(1, &state->text_vbo);

    glBindVertexArray(state->text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, state->text_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Init freetype library.
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        error_and_exit(EXIT_FAILURE, "Could not init freetype.");
    }

    // Load font face.
    if (FT_New_Face(ft, "./assets/8-BIT_WONDER.TTF", 0, &face))
    {
        error_and_exit(EXIT_FAILURE, "Could not load font.");
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    g = face->glyph;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (u32 i = 0, n = 0; i < 128; ++i)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
        {
            printf("Failed to load glyph '%c'\n", i);
            continue;
        }

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

    glGenVertexArrays(1, &state->text_vao);
    glGenBuffers(1, &state->text_vbo);
    glBindVertexArray(state->text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, state->text_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Setup projection matrix for each shader.
    mat4x4_ortho(state->projection, 0, WIDTH, 0, HEIGHT, -2.0f, 2.0f);

    mat4x4 text_projection;
    mat4x4_identity(text_projection);
    mat4x4_ortho(text_projection, 0, WIDTH * SCALE, 0, HEIGHT * SCALE, -2.0f, 2.0f);
    glUseProgram(state->text_shader);
    glUniformMatrix4fv(glGetUniformLocation(state->text_shader, "projection"), 1, GL_FALSE, &text_projection[0][0]);
}

void render_text(const char *text, float x, float y, vec4 color, u8 is_centered)
{
    glUseProgram(state->text_shader);

    glUniform4fv(glGetUniformLocation(state->text_shader, "color"), 1, color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(state->text_vao);

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
        glBindBuffer(GL_ARRAY_BUFFER, state->text_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (cd.advance_x / 64);
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}