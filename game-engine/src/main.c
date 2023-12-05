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


static Mix_Music *MUSIC_STAGE_1; 
static Mix_Chunk *SOUND_JUMP;


static const float SPEED_ENEMY_LARGE = 200;
static const float SPEED_ENEMY_SMALL = 4000;
static const float HEALTH_ENEMY_LARGE = 7;
static const float HEALTH_ENEMY_SMALL = 3;

typedef enum collision_layer
{
    COLLISON_LAYER_PLAYER = 1,
    COLLISON_LAYER_ENEMY = 1 << 1,
    COLLISON_LAYER_TERRAIN = 1 << 2,
} Collision_Layer;

vec4 player_color = {0, 1, 1, 1};

bool player_is_grounded = false;

static vec2 pos;

static void input_handle(Body *body_player)
{
    if (global.input.escape || global.input.start_controller)
        global.should_quit = true;

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
        audio_sound_play(SOUND_JUMP);
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

/*Pour les flammes dans lesquelles tombent les ennemis*/
void fire_on_hit(Body *self, Body *other, Hit hit)
{
    if (other->collision_layer == COLLISON_LAYER_ENEMY)
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

int main(int argc, char *argv[])
{
    time_init(60);
    config_init();
    SDL_Window *window = render_init();
    controller_init();
    physics_init();
    entity_init();
    animation_init();
    render_text_init();
    audio_init();
    scenes_init();
    
    
    
    
    //POUR LA MUSIQUE
    audio_sound_load(&SOUND_JUMP, "assets/jump.wav");
	audio_music_load(&MUSIC_STAGE_1, "assets/breezys_mega_quest_2_stage_1.mp3");
	audio_music_play(MUSIC_STAGE_1);

    

    MyCurlHandle curl_handle;

    mycurl_init(&curl_handle);

    SDL_ShowCursor(false); // Cacher le curseur

    u8 ennemy_mask = COLLISON_LAYER_ENEMY | COLLISON_LAYER_TERRAIN;
    u8 player_mask = COLLISON_LAYER_ENEMY | COLLISON_LAYER_TERRAIN;
    u8 fire_mask = COLLISON_LAYER_ENEMY | COLLISON_LAYER_PLAYER;

    size_t player_id = entity_create((vec2){100, 200}, // Postion (x,y)
                                     (vec2){24, 24},   // Size
                                     (vec2){0, 0},     // Velocité
                                     COLLISON_LAYER_PLAYER,
                                     player_mask,
                                     false,               // is_kinematic flag
                                     player_on_hit,       // on hit
                                     player_on_hit_static // on hit static
    );

    i32 window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);

    float width = window_width / render_get_scale();
    float height = window_height / render_get_scale();

    // Murs

    // Mur du haut
    u32 static_body_a_id = physics_static_body_create(
        (vec2){width * 0.5 - 12.5, height - 12.5}, // Position
        (vec2){width - 25, 25},                    // Size
        COLLISON_LAYER_TERRAIN);                   // Collision_layer
    // Mur de droite
    u32 static_body_b_id = physics_static_body_create(
        (vec2){width - 12.5, height * 0.5 + 12.5}, // position
        (vec2){25, height - 25},                   // taille
        COLLISON_LAYER_TERRAIN);
    // Mur du bas
    u32 static_body_c_id = physics_static_body_create(
        (vec2){width * 0.5 + 12.5, 12.5}, // Postion
        (vec2){width - 25, 25},           // Taille
        COLLISON_LAYER_TERRAIN);
    // Mur de gauche
    u32 static_body_d_id = physics_static_body_create(
        (vec2){12.5, height * 0.5 - 12.5}, // Position
        (vec2){25, height - 25},           // Taille
        COLLISON_LAYER_TERRAIN);

    u32 static_body_e_id = physics_static_body_create(
        (vec2){width * 0.5, height * 0.65},
        (vec2){62.5, 62.5},
        COLLISON_LAYER_TERRAIN);
    // Plateformes

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
    size_t entity_a_id = entity_create(
        (vec2){200, 200}, // Position (x,y)
        (vec2){25, 25},   // Size
        (vec2){400, 0},   // Velocité ( x , y )
        COLLISON_LAYER_ENEMY,
        ennemy_mask,
        false,
        NULL,                  // On hit another entity
        ennemy_on_hit_static); // On hit static body

    size_t entity_b_id = entity_create(
        (vec2){300, 300}, // Position (x,y)
        (vec2){25, 25},   // Size
        (vec2){400, 0},   // Velocité (x,y)
        COLLISON_LAYER_ENEMY,
        ennemy_mask,
        false,
        NULL,
        ennemy_on_hit_static);

    // size_t entity_fire = entity_create(
    //     (vec2){370, 50}, // Position (x,y)
    //     (vec2){25, 25},  // Size
    //     (vec2){0},       // Velocité (x,y)
    //     0,
    //     fire_mask,
    //     true,
    //     fire_on_hit,
    //     NULL);

    Sprite_Sheet sprite_sheet_player;
    render_sprite_sheet_init(&sprite_sheet_player, "assets/player.png", 24, 24); // Charger spritesheet joueur
    // render_sprite_sheet_init(&sprite_sheet_player, "assets/player_2.png", 55, 37); // Charger spritesheet joueur

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
  
    float spawn_timer = 0;

    while (!global.should_quit)
    {
        time_update();
        if (global.current_screen == MENU_SCREEN)
        {

            display_menu(window);
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

            /*Pour garder une trace de l'apparitions*/

            // Spawn enemies =>Faire apparaître des ennemis
            // {
            //     spawn_timer -= global.time.delta;
            //     if (spawn_timer <= 0)
            //     {
            //         spawn_timer = (float)((rand() % 200) + 200) / 100.f;
            //         spawn_timer *= 0.2;

            //         for (u32 i = 0; i < 50; i++)

            //         {

            //             bool is_flipped = rand() % 100 >= 50;

            //             float spawn_x = is_flipped ? 540 : 100;

            //             size_t entity_id = entity_create(
            //                 (vec2){spawn_x, 200}, // Position (x,y)
            //                 (vec2){20, 20},       // Size
            //                 (vec2){0, 0},         // Velocité (x,y)
            //                 COLLISON_LAYER_ENEMY,
            //                 ennemy_mask,
            //                 false,
            //                 NULL,
            //                 enemy_small_on_hit_static);
            //             Entity *entity = entity_get(entity_id);
            //             Body *body = physics_body_get(entity->body_id);
            //             float speed = SPEED_ENEMY_SMALL * ((rand() % 100) * 0.01) + 100;
            //             body->velocity[0] = is_flipped ? -speed : speed;
            //         }
            //     }
            // }

            render_begin();
            for (size_t i = 0; i < entity_count(); ++i)
            {
                Entity *entity = entity_get(i);
                Body *body = physics_body_get(entity->body_id);

                if (body->is_active)
                {
                    render_aabb(body, TURQUOISE);
                }
                else
                {
                    render_aabb(body, RED);
                }
            }

            render_aabb((float *)static_body_a, WHITE);
            render_aabb((float *)static_body_b, WHITE);
            render_aabb((float *)static_body_c, WHITE);
            render_aabb((float *)static_body_d, WHITE);
            render_aabb((float *)static_body_e, WHITE);
            render_aabb((float *)static_body_platform_b, GREEN);
            render_aabb((float *)static_body_platform_c, GREEN);
            render_aabb((float *)static_body_platform_d, GREEN);
            render_aabb((float *)static_body_platform_e, GREEN);

            // if (player_color[0] != 0)
            // {
            //     render_aabb((float *)body_player, player_color);
            // }
            render_aabb((float *)body_player, player_color);

            // render_aabb((float *)physics_body_get(entity_get(entity_a_id)->body_id), RED);
            // render_aabb((float *)physics_body_get(entity_get(entity_b_id)->body_id), RED);

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
        }

        time_update_late();
    }

    return EXIT_SUCCESS;
}