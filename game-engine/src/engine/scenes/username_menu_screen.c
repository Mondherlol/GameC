#include <stdio.h>
#include <SDL2/SDL.h>
#include "../scenes.h"
#include "../render.h"
#include "../util.h"
#include "../global.h"
#include "../my_curl.h"

Image pseudoMenuImages[2];
int currentSelection_p = 0;

MyCurlHandle curl_handler;

float width;
float height;

void username_menu_init()
{

    init_image(&pseudoMenuImages[0], "assets/menu/fenetre_pseudo_selected_save.png");
    init_image(&pseudoMenuImages[1], "assets/menu/fenetre_pseudo_selected_quit.png");

    width = global.window_width / render_get_scale();

    height = global.window_height / render_get_scale();

    mycurl_init(&curl_handler);
}

void display_username_menu(SDL_Window *window, u32 texture_slots[16])
{

    render_image(&pseudoMenuImages[currentSelection_p],
                 (vec2){0, 0},                                                                                                                              // position
                 (vec2){pseudoMenuImages[currentSelection_p].width / render_get_scale(), pseudoMenuImages[currentSelection_p].height / render_get_scale()}, // taille
                 texture_slots);

    SDL_Event menuEvent;

    while (SDL_PollEvent(&menuEvent))
    {
        switch (menuEvent.type)
        {
        case SDL_KEYDOWN:
            switch (menuEvent.key.keysym.sym)
            {

            case SDLK_LEFT:
                currentSelection_p = 0;
                break;
            case SDLK_ESCAPE:
                global.current_screen = MENU_SCREEN;
                break;
            case SDLK_RIGHT:
                currentSelection_p = 1;
                break;
            case SDLK_RETURN:
                switch (currentSelection_p)
                {
                case 0:
                    global.current_screen = MENU_SCREEN;
                    break;
                case 1:
                    global.current_screen = MENU_SCREEN;
                    break;
                }
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    render_textures(texture_slots);

    render_text(global.username, (width / 2), (height / 2) - 6, WHITE, 1);

    render_end(window);
}
