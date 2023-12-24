#include "../input.h"
#include "../global.h"
#include "../types.h"
#include "../util.h"

#define JOYSTICK_AXIS_LEFT_RIGHT 0
#define JOYSTICK_DEADZONE 8000 // Zone morte du joystick
#define JOYSTICK_BUTTON_LEFT 13
#define JOYSTICK_BUTTON_RIGHT 14
#define JOYSTICK_BUTTON_JUMP 1
#define JOYSTICK_BUTTON_SHOOT 2
#define JOYSTICK_BUTTON_START 6

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
    update_key_state(keyboard_state[global.config.keybinds[INPUT_KEY_SHOOT]], &global.input.shoot);

    // Si manette
    if (SDL_NumJoysticks() > 0)
    {
        SDL_Joystick *joystick = SDL_JoystickOpen(0); // On prend la premiere connectée

        if (joystick)
        {
            // Joystick de gauche
            update_key_state(SDL_JoystickGetAxis(joystick, JOYSTICK_AXIS_LEFT_RIGHT) < -JOYSTICK_DEADZONE, &global.input.joystick_left_controller);
            update_key_state(SDL_JoystickGetAxis(joystick, JOYSTICK_AXIS_LEFT_RIGHT) > JOYSTICK_DEADZONE, &global.input.joystick_right_controller);
            // Les boutons directionnels
            update_key_state(SDL_JoystickGetButton(joystick, JOYSTICK_BUTTON_LEFT), &global.input.left_controller);
            update_key_state(SDL_JoystickGetButton(joystick, JOYSTICK_BUTTON_RIGHT), &global.input.right_controller);
            // Le reste
            update_key_state(SDL_JoystickGetButton(joystick, JOYSTICK_BUTTON_JUMP), &global.input.jump_controller);
            update_key_state(SDL_JoystickGetButton(joystick, JOYSTICK_BUTTON_START), &global.input.start_controller);
            update_key_state(SDL_JoystickGetButton(joystick, JOYSTICK_BUTTON_SHOOT), &global.input.shoot_controller);

            SDL_JoystickClose(joystick);
        }
    }
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