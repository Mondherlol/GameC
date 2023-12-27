#include <stdio.h>
#include <SDL2/SDL.h>
#include "../scenes.h"
#include "../render.h"
#include "../util.h"
#include "../global.h"
#include "../my_curl.h"
#include "../audio.h"

float debug_menu_pos_x = 50;
float debug_menu_pos_y = 50;

#define MENU_ITEMS_COUNT 3

static Mix_Chunk *SOUND_SELECTED_BUTTON;

// tab pour stocker les images
Image menuImages[MENU_ITEMS_COUNT];
Image username_background;
// Variable pour suivre la sélection actuelle dans le menu
int currentSelection = 0;

u8 texture_slots[16] = {0};

// Obtenir les dimensions de la fenetre
float width;
float height;

void debug_username_menu()
{
    render_begin();

    render_image(&menuImages[currentSelection],
                 (vec2){0, 0},                                                                                                              // position
                 (vec2){menuImages[currentSelection].width / render_get_scale(), menuImages[currentSelection].height / render_get_scale()}, // taille
                 texture_slots);
}
void menu_init()
{
    audio_sound_load(&SOUND_SELECTED_BUTTON, "assets/audio/Select 1.wav");

    init_image(&menuImages[0], "assets/menu/1menu_selected_play.png");
    init_image(&menuImages[1], "assets/menu/2menu_selected_scores.png");
    init_image(&menuImages[2], "assets/menu/3menu_selected_quit.png");
    init_image(&username_background, "assets/menu/username_background.png");

    width = global.window_width / render_get_scale();

    height = global.window_height / render_get_scale();

    strcpy(global.generated_code, "");
}

#define SELECTION_DELAY 200 // Ajoutez cette constante pour définir la durée de la pause en millisecondes

static u32 lastSelectionTime = 0; // Ajoutez cette variable pour stocker le temps du dernier changement de sélection
static void menu_input_handle()
{
    Uint32 currentTime = SDL_GetTicks(); // Obtenez le temps actuel en millisecondes

    // Vérifiez si la durée depuis le dernier changement de sélection est supérieure à la pause définie
    if (currentTime - lastSelectionTime >= SELECTION_DELAY)
    {
        if (global.input.up || global.input.up_controller || global.input.joystick_up_controller)
        {
            currentSelection = (currentSelection - 1 + MENU_ITEMS_COUNT) % MENU_ITEMS_COUNT;
            lastSelectionTime = currentTime; // Mettez à jour le temps du dernier changement de sélection
            audio_sound_play(SOUND_SELECTED_BUTTON);
        }

        if (global.input.down || global.input.down_controller || global.input.joystick_down_controller)
        {
            currentSelection = (currentSelection + 1) % MENU_ITEMS_COUNT;
            lastSelectionTime = currentTime; // Mettez à jour le temps du dernier changement de sélection
            audio_sound_play(SOUND_SELECTED_BUTTON);
        }
    }
    if (global.input.confirm_controller)
    {
        // Gérer la sélection en fonction de currentSelection
        switch (currentSelection)
        {
        case 0:
            global.current_screen = GAME_SCREEN;
            break;
        case 1:
            global.current_screen = SCORE_SCREEN;
            score_reset();
            break;
        case 2:
            global.should_quit = true; // Quitter le jeu
            break;
        }
    }
}
void display_menu(SDL_Window *window)
{

    // initiation du rendu
    render_begin();

    render_image(&menuImages[currentSelection],
                 (vec2){0, 0},                                                                                                              // position
                 (vec2){menuImages[currentSelection].width / render_get_scale(), menuImages[currentSelection].height / render_get_scale()}, // taille
                 texture_slots);

    render_image(&username_background,
                 (vec2){475, 265},
                 (vec2){username_background.width / render_get_scale(), username_background.height / render_get_scale()},
                 texture_slots);

    menu_input_handle();

    if (global.current_screen == MENU_SCREEN)
    {
        SDL_Event menuEvent;

        while (SDL_PollEvent(&menuEvent))
        {

            switch (menuEvent.type)
            {
            case SDL_QUIT:
                global.should_quit = true;
                break;
            case SDL_JOYBUTTONDOWN:
                // Bouton de la manette enfoncé
                int buttonIndex = menuEvent.jbutton.button;
                printf(" Bouton %d appuyer", buttonIndex);
                break;
            case SDL_KEYDOWN:
                switch (menuEvent.key.keysym.sym)
                {
                case SDLK_p:
                    global.current_screen = USERNAME_MENU_SCREEN;
                    audio_sound_play(SOUND_SELECTED_BUTTON);
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
                        score_reset();
                        break;
                    case 2:
                        global.should_quit = true; // Quitter le jeu
                        break;
                    }
                    break;
                case SDLK_d:
                    // Entrer en mode débogage
                    printf("Enter debug mode. Type 'exit' to exit debug mode.\n");
                    while (1)
                    {
                        char input[100];
                        printf("Enter command (x , y, exit): ");
                        scanf("%s", input);

                        if (strcmp(input, "exit") == 0)
                            break;
                        else if (strcmp(input, "x") == 0)
                        {
                            printf("Enter new value for pos_x: ");
                            scanf("%f", &debug_menu_pos_x);
                            render_end(window);
                            debug_username_menu();
                            render_end(window);
                        }
                        else if (strcmp(input, "y") == 0)
                        {
                            printf("Enter new value for pos_y: ");
                            scanf("%f", &debug_menu_pos_y);
                            render_end(window);
                            debug_username_menu();
                            render_end(window);
                        }
                        else
                        {
                            printf("Unknown command. Try again.\n");
                        }
                    }
                    printf("Exit debug mode.\n");
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }
    }

    if (global.current_screen == MENU_SCREEN)
    {
        render_textures(texture_slots);
        render_text(global.username, 571, 306, WHITE, 1);
        render_text(global.generated_code, 55, height * 0.86, YELLOW, 1);
        render_end(window);
    }
    else if (global.current_screen == USERNAME_MENU_SCREEN)
    {
        display_username_menu(window, texture_slots);
    }
}
