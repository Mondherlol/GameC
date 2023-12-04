#include <stdio.h>
#include <SDL2/SDL.h>
#include "../menu.h"
#include "../render.h"
#include "../util.h"
#include "../global.h"

void display_menu(SDL_Window *window)
{

    render_begin();
    float width = global.window_width / render_get_scale();
    float height = global.window_height / render_get_scale();

    // Sprite_Sheet sprite_sheet_menu;
    // render_sprite_sheet_init(&sprite_sheet_menu, "assets/player_2.png", 1280, 720); // Charger spritesheet joueur
    // render_sprite_sheet_frame(&sprite_sheet_menu, 0, 0, (vec2){width / 2, height / 2}, false);

    // render_image(width, height, "./assets/player.png", 0, 0);

    render_text("Game Menu", width / 2, height * 0.8, WHITE, 1);
    render_text("1. Start Game", width / 2, height * 0.5, WHITE, 1);
    render_text("2. Options", width / 2, height * 0.4, WHITE, 1);
    render_text("3. Quit", width / 2, height * 0.3, WHITE, 1);

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
                // Code pour démarrer le jeu
                game_started = true;
                break;
            case SDLK_2:
                // Code pour afficher les options (à implémenter)
                break;
            case SDLK_3:
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

    // render_end(window, sprite_sheet_menu.texture_id);

    SDL_GL_SwapWindow(window); // Mettre à jour la fenêtre avec le rendu OPENGL
}