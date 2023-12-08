#include <stdio.h>
#include "../scenes.h"
#include "../global.h"

void scenes_init()
{
    global.current_screen = MENU_SCREEN;
    menu_init();
    score_init();
}
