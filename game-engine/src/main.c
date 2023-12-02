#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "engine/util.h"
#include "engine/global.h"
#include "engine/config.h"
#include "engine/time.h"
#include "engine/input.h"
#include "engine/physics.h"
#include "engine/entity.h"
#include "engine/render.h"
#include "engine/animation.h"

typedef enum collision_layer
{
    COLLISON_LAYER_PLAYER = 1,
    COLLISON_LAYER_ENEMY = 1 << 1,
    COLLISON_LAYER_TERRAIN = 1 << 2,
} Collision_Layer;

static bool should_quit = false;
vec4 player_color = {0, 1, 1, 1};
bool player_is_grounded = false;

static vec2 pos;

static void input_handle(Body *body_player)
{
    if (global.input.escape || global.input.start_controller)
        should_quit = true;

    float velx = 0;
    float vely = body_player->velocity[1];

    if (global.input.right || global.input.right_controller || global.input.joystick_right_controller)
    {
        velx += 400;
    }

    if (global.input.left || global.input.left_controller || global.input.joystick_left_controller)
    {
        velx -= 400;
    }

    if ((global.input.up || global.input.jump_controller) && player_is_grounded)
    {
        player_is_grounded = false;
        vely = 1200;
    }

    body_player->velocity[0] = velx;
    body_player->velocity[1] = vely;
}

void player_on_hit(Body *self, Body *other, Hit hit)
{
    if (other->collision_layer == COLLISON_LAYER_ENEMY)
    {
        player_color[0] = 1;
        player_color[2] = 0;
    }
}

void player_on_hit_static(Body *self, Static_Body *other, Hit hit)
{
    if (hit.normal[1] > 0)
    {
        player_is_grounded = true;
    }
}

void ennemy_on_hit_static(Body *self, Static_Body *other, Hit hit)
{
    if (hit.normal[0] > 0) // Le fait changer de direction lorsqu'il cogne un mur
    {
        self->velocity[0] = 400;
    }

    if (hit.normal[0] < 0)
    {
        self->velocity[0] = -400;
    }
}

int main(int argc, char *argv[])
{

    time_init(60);
    config_init();
    SDL_Window *window = render_init();
    controller_init();
    physics_init();
    entity_init();
    animation_init();

    SDL_ShowCursor(false); // Cacher le curseur

    u8 ennemy_mask = COLLISON_LAYER_ENEMY | COLLISON_LAYER_TERRAIN;
    u8 player_mask = COLLISON_LAYER_ENEMY | COLLISON_LAYER_TERRAIN;

    size_t player_id = entity_create((vec2){100, 200},
                                     (vec2){24, 24},
                                     (vec2){0, 0},
                                     COLLISON_LAYER_PLAYER,
                                     player_mask,
                                     player_on_hit,
                                     player_on_hit_static);

    i32 window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);
    float width = window_width / render_get_scale();
    float height = window_height / render_get_scale();

    // Murs
    u32 static_body_a_id = physics_static_body_create((vec2){width * 0.5 - 12.5, height - 12.5}, (vec2){width - 25, 25}, COLLISON_LAYER_TERRAIN);
    u32 static_body_b_id = physics_static_body_create((vec2){width - 12.5, height * 0.5 + 12.5}, (vec2){25, height - 25}, COLLISON_LAYER_TERRAIN);
    u32 static_body_c_id = physics_static_body_create((vec2){width * 0.5 + 12.5, 12.5}, (vec2){width - 25, 25}, COLLISON_LAYER_TERRAIN);
    u32 static_body_d_id = physics_static_body_create((vec2){12.5, height * 0.5 - 12.5}, (vec2){25, height - 25}, COLLISON_LAYER_TERRAIN);

    // Plateformes
    u32 static_body_e_id = physics_static_body_create(
        (vec2){width * 0.5, height * 0.65},
        (vec2){62.5, 62.5},
        COLLISON_LAYER_TERRAIN);

    u32 platform_b_id = physics_static_body_create(
        (vec2){width * 0.1, height * 0.25}, // Position
        (vec2){60, 10},                     // Taille
        COLLISON_LAYER_TERRAIN              // Masque de collision
    );

    u32 platform_c_id = physics_static_body_create(
        (vec2){width * 0.9, height * 0.25}, // Position
        (vec2){60, 10},                     // Taille
        COLLISON_LAYER_TERRAIN              // Masque de collision
    );

    u32 platform_d_id = physics_static_body_create(
        (vec2){width * 0.4, height * 0.43}, // Position
        (vec2){50, 10},                     // Taille
        COLLISON_LAYER_TERRAIN              // Masque de collision
    );

    u32 platform_e_id = physics_static_body_create(
        (vec2){width * 0.6, height * 0.43}, // Position
        (vec2){50, 10},                     // Taille
        COLLISON_LAYER_TERRAIN              // Masque de collision
    );
    // Ennemies
    size_t entity_a_id = entity_create((vec2){200, 200}, (vec2){25, 25}, (vec2){400, 0}, COLLISON_LAYER_ENEMY, ennemy_mask, NULL, ennemy_on_hit_static);
    size_t entity_b_id = entity_create((vec2){300, 300}, (vec2){25, 25}, (vec2){400, 0}, COLLISON_LAYER_ENEMY, ennemy_mask, NULL, ennemy_on_hit_static);

    Sprite_Sheet sprite_sheet_player;
    render_sprite_sheet_init(&sprite_sheet_player, "assets/player.png", 24, 24); // Charger spritesheet joueur

    // Definir animation de deplacement
    size_t adef_player_walk_id = animation_definition_create(
        &sprite_sheet_player,                         // Spritesheet
        (float[]){0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1}, // Durée
        (u8[]){0, 0, 0, 0, 0, 0, 0},                  // Ligne
        (u8[]){1, 2, 3, 4, 5, 6, 7},                  // Colonne
        7                                             // Nbr de frames
    );

    // Definir animation d'idle
    size_t adef_player_idle_id = animation_definition_create(
        &sprite_sheet_player, // Spritesheet
        (float[]){0},         // Durée
        (u8[]){2},            // Ligne
        (u8[]){0},            // colonne
        1                     // Nombre de frames
    );
    //                                             Def animation      loop ?
    size_t anim_player_walk_id = animation_create(adef_player_walk_id, true);
    size_t anim_player_idle_id = animation_create(adef_player_idle_id, false);

    Entity *player = entity_get(player_id);
    player->animation_id = anim_player_idle_id;

    while (!should_quit)
    {

        time_update();

        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                should_quit = true;
                break;
            case SDL_KEYDOWN:
                // Vérifier si la touche "r" est pressée
                if (event.key.keysym.sym == SDLK_r)
                {
                    printf("Touche R pressee");
                }
                break;
            default:
                break;
            }
        }

        Entity *player = entity_get(player_id);
        Body *body_player = physics_body_get(player->body_id);

        if (body_player->velocity[0] != 0)
        {
            player->animation_id = anim_player_walk_id;
        }
        else
        {
            player->animation_id = anim_player_idle_id;
        }

        Static_Body *static_body_a = physics_static_body_get(static_body_a_id);
        Static_Body *static_body_b = physics_static_body_get(static_body_b_id);
        Static_Body *static_body_c = physics_static_body_get(static_body_c_id);
        Static_Body *static_body_d = physics_static_body_get(static_body_d_id);
        Static_Body *static_body_e = physics_static_body_get(static_body_e_id);

        Static_Body *static_body_platform_b = physics_static_body_get(platform_b_id);
        Static_Body *static_body_platform_c = physics_static_body_get(platform_c_id);
        Static_Body *static_body_platform_d = physics_static_body_get(platform_d_id);
        Static_Body *static_body_platform_e = physics_static_body_get(platform_e_id);

        input_update();
        input_handle(body_player);
        physics_update();
        animation_update(global.time.delta);

        render_begin();

        render_aabb((float *)static_body_a, WHITE);
        render_aabb((float *)static_body_b, WHITE);
        render_aabb((float *)static_body_c, WHITE);
        render_aabb((float *)static_body_d, WHITE);
        render_aabb((float *)static_body_e, WHITE);
        render_aabb((float *)static_body_platform_b, GREEN);
        render_aabb((float *)static_body_platform_c, GREEN);
        render_aabb((float *)static_body_platform_d, GREEN);
        render_aabb((float *)static_body_platform_e, GREEN);

        if (player_color[0] != 0)
        {
            render_aabb((float *)body_player, player_color);
        }

        render_aabb((float *)physics_body_get(entity_get(entity_a_id)->body_id), RED);
        render_aabb((float *)physics_body_get(entity_get(entity_b_id)->body_id), RED);

        for (size_t i = 0; i < entity_count(); ++i)
        {
            Entity *entity = entity_get(i);

            if (!entity->is_active)
            {
                continue;
            }

            if (entity->animation_id == (size_t)-1)
            {
                continue;
            }

            Body *body = physics_body_get(entity->body_id);
            Animation *anim = animation_get(entity->animation_id);

            Animation_Definition *adef = anim->definition;
            Animation_Frame *aframe = &adef->frames[anim->current_frame_index];

            if (body->velocity[0] < 0)
            {
                anim->is_flipped = true;
            }
            else if (body->velocity[0] > 0)
            {
                anim->is_flipped = false;
            }

            render_sprite_sheet_frame(adef->sprite_sheet, aframe->row, aframe->column, body->aabb.position, anim->is_flipped);
        }

        render_end(window, sprite_sheet_player.texture_id);
        player_color[0] = 0;
        player_color[2] = 1;

        time_update_late();
    }

    return EXIT_SUCCESS;
}
