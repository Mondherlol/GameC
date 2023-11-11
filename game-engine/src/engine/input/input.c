#include "../input.h"
#include "../global.h"
#include "../types.h"
#include "../util.h"

// Gérer l'état des touches pressées
static void update_key_state(u8 current_state, Key_State *key_state)
{
    if (current_state)
    {
        if (*key_state > 0)
            *key_state = KS_HELD;
        else
            *key_state = KS_PRESSED;
    }
    else
    {
        *key_state = KS_UNPRESSED;
    }
}

void input_update()
{
    // Obtient l'état actuel du clavier
    const u8 *keyboard_state = SDL_GetKeyboardState(NULL);

    // Met à jour l'état de chaque touche en fonction de la config
    update_key_state(keyboard_state[global.config.keybinds[INPUT_KEY_LEFT]], &global.input.left);
    update_key_state(keyboard_state[global.config.keybinds[INPUT_KEY_RIGHT]], &global.input.right);
    update_key_state(keyboard_state[global.config.keybinds[INPUT_KEY_UP]], &global.input.up);
    update_key_state(keyboard_state[global.config.keybinds[INPUT_KEY_DOWN]], &global.input.down);
    update_key_state(keyboard_state[global.config.keybinds[INPUT_KEY_ESCAPE]], &global.input.escape);
}

void controller_init()
{

    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0)
        ERROR_RETURN("Erreur lors de l'initialisation de la manette : ", "");

    int numControllers = SDL_NumJoysticks();
    if (numControllers < 1)
    {
        printf("Aucune manette connectée");
    }
    else
    {
        // Acceder a la premiere manette
        SDL_Joystick *gameController = SDL_JoystickOpen(0);
        if (gameController == NULL)
        {
            ERROR_RETURN("Erreur lors de la connexion avec la manette : %s\n", SDL_GetError());
        }
    }
}