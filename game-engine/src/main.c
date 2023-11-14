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

static void input_handle(void)
{
    // if (global.input.left == KS_PRESSED || global.input.left == KS_HELD)
    //     pos[0] -= 500 * global.time.delta;
    // if (global.input.right == KS_PRESSED || global.input.right == KS_HELD)
    //     pos[0] += 500 * global.time.delta;
    // if (global.input.up == KS_PRESSED || global.input.up == KS_HELD)
    //     pos[1] += 500 * global.time.delta;
    // if (global.input.down == KS_PRESSED || global.input.down == KS_HELD)
    //     pos[1] -= 500 * global.time.delta;
    if (global.input.escape == KS_PRESSED || global.input.escape == KS_HELD)
        should_quit = true;

    i32 x, y;
    SDL_GetMouseState(&x, &y);
    pos[0] = (float)x;
    pos[1] = global.render.height - y;
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

    pos[0] = global.render.width * 0.5;
    pos[1] = global.render.height * 0.5;

    AABB test_aabb = {
        .position = {pos[0], pos[1]},
        .half_size = {50, 50}};

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
        input_handle();
        physics_update();

        render_begin();

        render_aabb((float *)&test_aabb, (vec4){1, 1, 1, 0.5});

        // Un quad qui suit le curseur et devient rouge quand entre en colision avec le aabb
        render_quad(
            pos,
            (vec2){10, 10},
            physics_point_intersect_aabb(pos, test_aabb) ? RED : WHITE);

        render_end();
        time_update_late();
    }
    return EXIT_SUCCESS;
}
