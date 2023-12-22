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
#include "engine/my_curl.h"
#include "engine/scenes.h"
#include "engine/audio.h"

void reset(void);

#define DEBUG 1

static Mix_Music *MUSIC_STAGE_1;
static Mix_Chunk *SOUND_JUMP;

static Sprite_Sheet sprite_sheet_player;
static Sprite_Sheet sprite_sheet_map;

static const float SPEED_PLAYER = 250;
static const float JUMP_VELOCITY = 1350;
static const float GROUNDED_TIME = 0.1; // Temps passé au sol
static const float SPEED_ENEMY_LARGE = 200;
static const float SPEED_ENEMY_SMALL = 4000;
static const float HEALTH_ENEMY_LARGE = 7;
static const float HEALTH_ENEMY_SMALL = 3;

typedef enum collision_layer
{
    COLLISION_LAYER_PLAYER = 1,
    COLLISION_LAYER_ENEMY = 1 << 1,
    COLLISION_LAYER_TERRAIN = 1 << 2,
} Collision_Layer;

static const u8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
static const u8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;
static const u8 fire_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_PLAYER;

static SDL_Window *window;
static size_t anim_player_walk_id;
static size_t anim_player_idle_id;

// States
bool player_is_grounded = false;
static vec2 pos;
static float spawn_timer = 0;
static float ground_timer = 0;

// Debugage
static u32 texture_slots[8] = {0};
static size_t player_id;

static void input_handle(Body *body_player)
{
    if (global.input.escape || global.input.start_controller)
        global.should_quit = true;

    Animation *walk_anim = animation_get(anim_player_walk_id);
    Animation *idle_anim = animation_get(anim_player_idle_id);

    float velx = 0;
    float vely = body_player->velocity[1];

    if (global.input.right || global.input.right_controller || global.input.joystick_right_controller)
    {
        velx += SPEED_PLAYER;
        walk_anim->is_flipped = false;
        idle_anim->is_flipped = false;
    }

    if (global.input.left || global.input.left_controller || global.input.joystick_left_controller)
    {
        velx -= SPEED_PLAYER;
        walk_anim->is_flipped = true;
        idle_anim->is_flipped = true;
    }

    if ((global.input.up || global.input.jump_controller) && player_is_grounded)
    {
        player_is_grounded = false;
        vely = JUMP_VELOCITY;
        audio_sound_play(SOUND_JUMP);
    }

    body_player->velocity[0] = velx;
    body_player->velocity[1] = vely;
}

void player_on_hit(Body *self, Body *other, Hit hit)
{
    if (other->collision_layer == COLLISION_LAYER_ENEMY)
    {
        // Faire quelque chose si le joueur touche l'ennemi
    }
}

void player_on_hit_static(Body *self, Static_Body *other, Hit hit)
{
    if (hit.normal[1] > 0)
    {
        player_is_grounded = true;
    }
}

/**/
void enemy_small_on_hit_static(Body *self, Static_Body *other, Hit hit)
{
    if (hit.normal[0] > 0)
    {
        self->velocity[0] = SPEED_ENEMY_SMALL;
    }

    if (hit.normal[0] < 0)
    {
        self->velocity[0] = -SPEED_ENEMY_SMALL;
    }
}
/**/
void enemy_large_on_hit_static(Body *self, Static_Body *other, Hit hit)
{
    if (hit.normal[0] > 0)
    {
        self->velocity[0] = SPEED_ENEMY_LARGE;
    }

    if (hit.normal[0] < 0)
    {
        self->velocity[0] = -SPEED_ENEMY_LARGE;
    }
}

/*Pour la flamme dans laquelle tombe l'ennemi*/
void fire_on_hit(Body *self, Body *other, Hit hit)
{
    if (other->collision_layer == COLLISION_LAYER_ENEMY)
    {
        for (size_t i = 0; i < entity_count(); i++)
        {
            Entity *entity = entity_get(i);

            if (entity->body_id == hit.other_id)
            {
                Body *body = physics_body_get(entity->body_id);
                body->is_active = false;
                entity->is_active = false;
                break;
            }
        }
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

void reset(void)
{
    audio_music_play(MUSIC_STAGE_1);

    physics_reset();
    entity_reset();

    ground_timer = 0;
    spawn_timer = 0;

    float width = global.window_width / render_get_scale();
    float height = global.window_height / render_get_scale();

    player_id = entity_create((vec2){100, 200}, // Postion (x,y)
                              (vec2){24, 24},   // Size
                              (vec2){0, 6},     // Offset sprite
                              (vec2){0, 0},     // Velocité
                              COLLISION_LAYER_PLAYER,
                              player_mask,
                              false, // is_kinematic flag
                              (size_t)-1,
                              player_on_hit,       // on hit
                              player_on_hit_static // on hit static
    );
    // Initialiser terrain du  niveau
    {

        physics_static_body_create((vec2){width * 0.5, height - 16}, (vec2){width, 32}, COLLISION_LAYER_TERRAIN);         // Plafond
        physics_static_body_create((vec2){width * 0.25 - 16, 16}, (vec2){width * 0.5 - 32, 48}, COLLISION_LAYER_TERRAIN); // Sol gauche
        physics_static_body_create((vec2){width * 0.75 + 16, 16}, (vec2){width * 0.5 - 32, 48}, COLLISION_LAYER_TERRAIN); // Sol droite
        physics_static_body_create((vec2){16, height * 0.5 - 3 * 32}, (vec2){32, height}, COLLISION_LAYER_TERRAIN);
        physics_static_body_create((vec2){width - 16, height * 0.5 - 3 * 32}, (vec2){32, height}, COLLISION_LAYER_TERRAIN);
        physics_static_body_create((vec2){32 + 64, height - 32 * 3 - 16}, (vec2){128, 32}, COLLISION_LAYER_TERRAIN);
        physics_static_body_create((vec2){width - 32 - 64, height - 32 * 3 - 16}, (vec2){128, 32}, COLLISION_LAYER_TERRAIN);
        physics_static_body_create((vec2){width * 0.5, height - 32 * 3 - 16}, (vec2){192, 32}, COLLISION_LAYER_TERRAIN);
        physics_static_body_create((vec2){width * 0.5, 32 * 3 + 24}, (vec2){448, 32}, COLLISION_LAYER_TERRAIN);
        physics_static_body_create((vec2){16, height - 64}, (vec2){32, 64}, COLLISION_LAYER_TERRAIN);         // Spawn ennemis de gauche
        physics_static_body_create((vec2){width - 16, height - 64}, (vec2){32, 64}, COLLISION_LAYER_TERRAIN); // Spawn ennemis de droite

        // Reste à creer le feu.
    }
    // Entity *player = entity_get(0);
    // player->animation_id = anim_player_idle_id;
}

int main(int argc, char *argv[])
{

    time_init(60);
    SDL_Window *window = render_init();

    config_init();
    // controller_init();
    physics_init();
    entity_init();
    animation_init();
    render_text_init();
    audio_init();
    scenes_init();
    score_init();

    SDL_ShowCursor(false); // Cacher le curseur

    // Initialiser audio
    {
        audio_sound_load(&SOUND_JUMP, "assets/jump.wav");
        audio_music_load(&MUSIC_STAGE_1, "assets/breezys_mega_quest_2_stage_1.mp3");
    }

    // Initialiser curl

    MyCurlHandle curl_handle;
    mycurl_init(&curl_handle);

    // Initialiser sprites
    {
        render_sprite_sheet_init(&sprite_sheet_player, "assets/player.png", 24, 24);
        render_sprite_sheet_init(&sprite_sheet_map, "assets/level_1_map.png", 640, 360);
    }

    // Initialiser animations
    {

        size_t adef_player_walk_id = animation_definition_create(&sprite_sheet_player, 0.1, 0, (u8[]){1, 2, 3, 4, 5, 6, 7}, 7);
        size_t adef_player_idle_id = animation_definition_create(&sprite_sheet_player, 0, 0, (u8[]){0}, 1);

        anim_player_walk_id = animation_create(adef_player_walk_id, true);
        anim_player_idle_id = animation_create(adef_player_idle_id, false);
    }

    reset();

    while (!global.should_quit)
    {
        time_update();
        if (global.current_screen == MENU_SCREEN)
        {
            display_menu(window);
        }
        else if (global.current_screen == SCORE_SCREEN)
        {
            display_score(window);
        }
        else
        {
            SDL_Event event;

            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_QUIT:
                    global.should_quit = true;
                    break;
                case SDL_KEYDOWN:
                    // Vérifier si la touche "r" est pressée
                    if (event.key.keysym.sym == SDLK_r)
                    {
                        printf("Test de la route /ping avec une requête GET dans un thread dédié.......\n");
                        // Créer une structure pour stocker les données de la requête
                        CurlRequestData *curlData = malloc(sizeof(CurlRequestData));
                        curlData->handle = &curl_handle;
                        curlData->endpoint = "/ping";

                        // Créer un thread pour effectuer la requête
                        HANDLE thread = CreateThread(NULL, 0, async_curl_request, curlData, 0, NULL);
                        CloseHandle(thread); // Fermer le handle du thread pour libérer ses ressources lorsqu'il a terminé
                    }
                    break;
                default:
                    break;
                }
            }

            // Mettre a jour les timers
            {
                spawn_timer -= global.time.delta;
                ground_timer -= global.time.delta;
            }

            Entity *player = entity_get(player_id);
            Body *body_player = physics_body_get(player->body_id);

            if (body_player->velocity[0] != 0 || (global.input.left || global.input.right))
            {
                player->animation_id = anim_player_walk_id;
            }
            else
            {
                player->animation_id = anim_player_idle_id;
            }

            input_update();
            input_handle(body_player);
            physics_update();

            animation_update(global.time.delta);

            render_begin();

            // #if DEBUG
            for (size_t i = 0; i < entity_count(); ++i)
            {
                Entity *entity = entity_get(i);
                Body *body = physics_body_get(entity->body_id);

                if (body->is_active)
                {
                    render_aabb((float *)body, TURQUOISE);
                }
                else
                {
                    render_aabb((float *)body, RED);
                }
            }

            for (size_t i = 0; i < physics_static_body_count(); ++i)
            {
                render_aabb((float *)physics_static_body_get(i), WHITE);
            }
            // #endif

            // Rendre le terrain
            render_sprite_sheet_frame(&sprite_sheet_map, 0, 0, (vec2){render_width / 2.0, render_height / 2.0}, false, (vec4){1, 1, 1, 1}, texture_slots);

            // Gerer l'animation des entity
            for (size_t i = 0; i < entity_count(); ++i)
            {
                Entity *entity = entity_get(i);
                if (!entity->is_active || entity->animation_id == (size_t)-1)
                {
                    continue;
                }

                Body *body = physics_body_get(entity->body_id);
                Animation *anim = animation_get(entity->animation_id);

                if (body->velocity[0] < 0)
                {
                    anim->is_flipped = true;
                }
                else if (body->velocity[0] > 0)
                {
                    anim->is_flipped = false;
                }

                vec2 pos;
                vec2_add(pos, body->aabb.position, entity->sprite_offset);
                animation_render(anim, pos, WHITE, texture_slots);
            }

            render_textures(texture_slots);
            render_text("joueur", body_player->aabb.position[0] - 5, body_player->aabb.position[1] + 20, WHITE, 1);

            render_end(window);
        }

        time_update_late();
    }

    return EXIT_SUCCESS;
}