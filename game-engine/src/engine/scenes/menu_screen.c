#include <stdio.h>
#include <SDL2/SDL.h>
#include "../scenes.h"
#include "../render.h"
#include "../util.h"
#include "../global.h"
#include "../my_curl.h"

#define MENU_ITEMS_COUNT 3

// tab pour stocker les images
Image menuImages[MENU_ITEMS_COUNT];
// Variable pour suivre la sélection actuelle dans le menu
int currentSelection = 0;

u8 texture_slots[8] = {0};

MyCurlHandle curl_handler;

// Obtenir les dimensions de la fenetre
float width;
float height;

void menu_init()
{

    init_image(&menuImages[0], "assets/1menu_selected_play.png");
    init_image(&menuImages[1], "assets/2menu_selected_scores.png");
    init_image(&menuImages[2], "assets/3menu_selected_quit.png");

    width = global.window_width / render_get_scale();

    height = global.window_height / render_get_scale();

    mycurl_init(&curl_handler);

    printf("Test de generation de code...\n");
    if (genererate_code(&curl_handler) != 0)
    {
        fprintf(stderr, "La requête pour generer le code a echouer.\n");
    };

    printf(" CODE GENERER = %s", global.generated_code);
}

void display_menu(SDL_Window *window)
{

    // initiation du rendu
    render_begin();

    // mise a jour de l'entree de lutilisateur
    input_update();

    render_image(&menuImages[currentSelection],
                 (vec2){0, 0},                                                                                                              // position
                 (vec2){menuImages[currentSelection].width / render_get_scale(), menuImages[currentSelection].height / render_get_scale()}, // taille
                 texture_slots);

    SDL_Event menuEvent;

    while (SDL_PollEvent(&menuEvent))
    {
        switch (menuEvent.type)
        {
        case SDL_QUIT:
            global.should_quit = true;
            break;
        case SDL_KEYDOWN:
            switch (menuEvent.key.keysym.sym)
            {

            case SDLK_UP:
                // Changer la sélection vers l'image précédente
                currentSelection = (currentSelection - 1 + MENU_ITEMS_COUNT) % MENU_ITEMS_COUNT;
                break;
            case SDLK_DOWN:
                currentSelection = (currentSelection + 1) % MENU_ITEMS_COUNT;
                break;
            case SDLK_RETURN:
                // Gérer la sélection en fonction de currentSelection
                switch (currentSelection)
                {
                case 0:
                    global.current_screen = GAME_SCREEN;
                    break;
                case 1:
                    global.current_screen = SCORE_SCREEN;
                    break;
                case 2:
                    global.should_quit = true; // Quitter le jeu
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

    render_text(global.generated_code, 55, height * 0.86, YELLOW, 1);

    render_end(window);
}
