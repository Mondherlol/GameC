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

static bool should_quit = false;
static vec2 pos;

static void input_handle(Body *body_player)
{
    if (global.input.escape)
        should_quit = true;

    float velx = 0;
    float vely = body_player->velocity[1];

    if (global.input.right)
    {
        velx += 1000;
    }

    if (global.input.left)
    {
        velx -= 1000;
    }

    if (global.input.up)
    {
        vely = 4000;
    }

    if (global.input.down)
    {
        vely -= 800;
    }

    body_player->velocity[0] = velx;
    body_player->velocity[1] = vely;
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
    puts("Jeu compile avec succes !");
    time_init(60);
    config_init();
    render_init();
    controller_init();
    physics_init();

    SDL_ShowCursor(false); // Cacher le curseur

    float width = global.render.width;
    float height = global.render.height;

    u32 body_id = physics_body_create((vec2){width / 2, height / 3}, (vec2){50, 50});

    u32 static_body_a_id = physics_static_body_create((vec2){width * 0.5 - 25, height - 25}, (vec2){width - 50, 50});
    u32 static_body_b_id = physics_static_body_create((vec2){width - 25, height * 0.5 + 25}, (vec2){50, height - 50});
    u32 static_body_c_id = physics_static_body_create((vec2){width * 0.5 + 25, 25}, (vec2){width - 50, 50});
    u32 static_body_d_id = physics_static_body_create((vec2){25, height * 0.5 - 25}, (vec2){50, height - 50});
    u32 static_body_e_id = physics_static_body_create((vec2){width * 0.5, height * 0.5}, (vec2){150, 150});

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
                printf("Un bouton a ete appuiyé.");
                break;
            default:
                break;
            }
        }

        input_update();

        // On récupére les body à nouveau car si la liste est pleine on peut plus récupérer
        Body *body_player = physics_body_get(body_id);
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
        render_aabb((float *)body_player, CYAN);

        render_end();
        time_update_late();
    }
    return EXIT_SUCCESS;
}
