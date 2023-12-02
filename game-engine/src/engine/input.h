#pragma once

typedef enum input_key
{
    INPUT_KEY_LEFT,
    INPUT_KEY_RIGHT,
    INPUT_KEY_UP,
    INPUT_KEY_DOWN,
    INPUT_KEY_ESCAPE
} Input_Key;

typedef enum key_state
{
    KS_UNPRESSED,
    KS_PRESSED,
    KS_HELD
} Key_State;

typedef struct input_state
{
    // Le clavier
    Key_State left;
    Key_State right;
    Key_State up;
    Key_State down;
    Key_State escape;

    // La manette
    Key_State joystick_left_controller;
    Key_State joystick_right_controller;
    Key_State left_controller;
    Key_State right_controller;
    Key_State jump_controller;
    Key_State start_controller;

} Input_State;

void input_update(void);
void controller_init(void);