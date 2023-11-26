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
    if (global.input.escape)
        should_quit = true;

    float velx = 0;
    float vely = body_player->velocity[1];

    if (global.input.right)
    {
        velx += 600;
    }

    if (global.input.left)
    {
        velx -= 600;
    }

    if (global.input.up && player_is_grounded)
    {
        player_is_grounded = false;
        vely = 2000;
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

static void controller_handle(SDL_Event event)
{
    if (event.jaxis.axis == 0)
    {
        // Axe X
        int xAxisValue = event.jaxis.value;

        // Vérifier la "zone morte" autour de zéro
        if (abs(xAxisValue) > 8000)
        {
            pos[0] += 500 * global.time.delta * xAxisValue / 32767.0;
        }
    }
    else if (event.jaxis.axis == 1)
    {
        // Axe Y
        int yAxisValue = event.jaxis.value;

        if (abs(yAxisValue) > 8000)
        {
            pos[1] -= 500 * global.time.delta * yAxisValue / 32767.0;
        }
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

    u32 static_body_a_id = physics_static_body_create((vec2){width * 0.5 - 12.5, height - 12.5}, (vec2){width - 25, 25}, COLLISON_LAYER_TERRAIN);
    u32 static_body_b_id = physics_static_body_create((vec2){width - 12.5, height * 0.5 + 12.5}, (vec2){25, height - 25}, COLLISON_LAYER_TERRAIN);
    u32 static_body_c_id = physics_static_body_create((vec2){width * 0.5 + 12.5, 12.5}, (vec2){width - 25, 25}, COLLISON_LAYER_TERRAIN);
    u32 static_body_d_id = physics_static_body_create((vec2){12.5, height * 0.5 - 12.5}, (vec2){25, height - 25}, COLLISON_LAYER_TERRAIN);
    u32 static_body_e_id = physics_static_body_create((vec2){width * 0.5, height * 0.5}, (vec2){62.5, 62.5}, COLLISON_LAYER_TERRAIN);

    size_t entity_a_id = entity_create((vec2){200, 200}, (vec2){25, 25}, (vec2){400, 0}, COLLISON_LAYER_ENEMY, ennemy_mask, NULL, ennemy_on_hit_static);
    size_t entity_b_id = entity_create((vec2){300, 300}, (vec2){25, 25}, (vec2){400, 0}, COLLISON_LAYER_ENEMY, ennemy_mask, NULL, ennemy_on_hit_static);

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
            case SDL_JOYAXISMOTION:
                // Mouvement de l'axe de la manette
                controller_handle(event);
                break;

            case SDL_JOYBUTTONDOWN:
                // Bouton de la manette enfoncé
                int buttonIndex = event.jbutton.button;
                printf("Un bouton a ete appuyé.");
                break;
            default:
                break;
            }
        }

        input_update();

        // On récupére les body à nouveau car si la liste est pleine on peut plus récupérer

        Entity *player = entity_get(player_id);
        Body *body_player = physics_body_get(player->body_id);
        Static_Body *static_body_a = physics_static_body_get(static_body_a_id);
        Static_Body *static_body_b = physics_static_body_get(static_body_b_id);
        Static_Body *static_body_c = physics_static_body_get(static_body_c_id);
        Static_Body *static_body_d = physics_static_body_get(static_body_d_id);
        Static_Body *static_body_e = physics_static_body_get(static_body_e_id);

        input_handle(body_player);
        physics_update();

        render_begin();

        render_aabb((float *)static_body_a, WHITE);
        render_aabb((float *)static_body_b, WHITE);
        render_aabb((float *)static_body_c, WHITE);
        render_aabb((float *)static_body_d, WHITE);
        render_aabb((float *)static_body_e, WHITE);
        render_aabb((float *)body_player, player_color);

        render_aabb((float *)physics_body_get(entity_get(entity_a_id)->body_id), WHITE);
        render_aabb((float *)physics_body_get(entity_get(entity_b_id)->body_id), WHITE);

        // for (u32 i = 0; i < 10000; ++i)
        // {
        //     vec4 color = {
        //         (rand() % 255) / 255.0,
        //         (rand() % 255) / 255.0,
        //         (rand() % 255) / 255.0,
        //         (rand() % 255) / 255.0,
        //     };
        //     append_quad((vec2){rand() % 640, rand() % 360},
        //                 (vec2){rand() % 100, rand() % 100},
        //                 NULL,
        //                 color);
        // }
        

     
        render_end(window);
       
        player_color[0] = 0;
        player_color[2] = 1;

        time_update_late();
    }
    
    return EXIT_SUCCESS;
}
