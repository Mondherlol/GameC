#include <stdio.h>
#include <SDL2/SDL.h>
#include "../scenes.h"
#include "../render.h"
#include "../util.h"
#include "../global.h"

Image menuImage;

void menu_init()
{
    init_image(&menuImage, "assets/menu.png");
}

void display_menu(SDL_Window *window)
{

    render_begin();

    input_update();

    float width = global.window_width / render_get_scale();
    float height = global.window_height / render_get_scale();

    render_image(&menuImage,
                 (vec2){0, 0},                                                                       // position
                 (vec2){menuImage.width / render_get_scale(), menuImage.height / render_get_scale()} // Size
    );

    render_text("NON CONNECTE ", width / 2, height * 0.8, RED, 1);

    // render_text("Game Menu", width / 2, height * 0.8, WHITE, 1);
    // render_text("1. Start Game", width / 2, height * 0.5, WHITE, 1);
    // render_text("2. Options", width / 2, height * 0.4, WHITE, 1);
    // render_text("3. Quit", width / 2, height * 0.3, WHITE, 1);

    if (global.input.escape || global.input.start_controller)
        global.should_quit = true;

    SDL_Event menuEvent;
    while (SDL_PollEvent(&menuEvent))
    {

        switch (menuEvent.type)
        {
        case SDL_QUIT:
            global.should_quit = true; // Quitter le jeu si la fenêtre est fermée
            break;
        case SDL_KEYDOWN:
            switch (menuEvent.key.keysym.sym)
            {
            case SDLK_1:
                // Demarer le jeu
                global.current_screen = GAME_SCREEN;
                break;
            case SDLK_2:
                // Ouvrir les parametres
                global.current_screen = SETTINGS_SCREEN;

                break;
            case SDLK_3:
                // Quitter le jeu
                global.should_quit = true; // Quitter le jeu si "Quit" est sélectionné
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    SDL_GL_SwapWindow(window); // Mettre à jour la fenêtre avec le rendu OPENGL
}