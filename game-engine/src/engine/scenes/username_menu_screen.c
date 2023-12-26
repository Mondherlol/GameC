#include <stdio.h>
#include <SDL2/SDL.h>
#include "../scenes.h"
#include "../render.h"
#include "../util.h"
#include "../global.h"
#include "../my_curl.h"
#include "../config.h"
#include "../audio.h"

Image pseudoMenuImages[2];
int currentSelection_p = 0;

char current_username[6] = "";

static Mix_Chunk *SOUND_SELECTED_BUTTON;
static Mix_Chunk *SOUND_CANCEL_BUTTON;
static Mix_Chunk *SOUND_CONFIRM_BUTTON;

void username_menu_init()
{
    audio_sound_load(&SOUND_SELECTED_BUTTON,"assets/audio/Select 1.wav");
    audio_sound_load(&SOUND_CANCEL_BUTTON,"assets/audio/Cancel 1.wav");
    audio_sound_load(&SOUND_CONFIRM_BUTTON,"assets/audio/Confirm 1.wav");

    init_image(&pseudoMenuImages[0], "assets/menu/fenetre_pseudo_selected_save.png");
    init_image(&pseudoMenuImages[1], "assets/menu/fenetre_pseudo_selected_quit.png");
    strncpy(current_username, global.username, 6);
}

// Fonction pour mettre à jour le pseudo en cours de saisie
void update_current_username(SDL_Keycode key)
{
    if (strlen(current_username) < 6)
    {
        if ((key >= SDLK_a && key <= SDLK_z) || (key >= SDLK_0 && key <= SDLK_9))
        {
            // Ajoute la lettre ou le chiffre à la fin du pseudo en cours de saisie
            strncat(current_username, SDL_GetKeyName(key), 1);
        }
    }

    // Supprime le dernier caractère si la touche Backspace est enfoncée
    if (key == SDLK_BACKSPACE && strlen(current_username) > 0)
    {
        current_username[strlen(current_username) - 1] = '\0';
    }
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
                audio_sound_play(SOUND_CONFIRM_BUTTON);
                break;
            case SDLK_ESCAPE:
                global.current_screen = MENU_SCREEN;
                audio_sound_play(SOUND_SELECTED_BUTTON);
                break;
            case SDLK_RIGHT:
                currentSelection_p = 1;
                audio_sound_play(SOUND_CANCEL_BUTTON);
                break;
            case SDLK_RETURN:
                switch (currentSelection_p)
                {
                case 0:
                    save_username(current_username);
                    global.current_screen = MENU_SCREEN;
                    break;
                case 1:
                    global.current_screen = MENU_SCREEN;
                    break;
                }
                break;
            default:
                update_current_username(menuEvent.key.keysym.sym);
                break;
            }
            break;
        default:
            break;
        }
    }

    render_textures(texture_slots);

    render_text(current_username, (render_width / 2), (render_height / 2) - 6, WHITE, 1);

    render_end(window);
}
