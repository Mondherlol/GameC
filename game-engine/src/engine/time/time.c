#include <SDL2/SDL.h>
#include "../time.h"
#include "../global.h"

void time_init(u32 frame_rate)
{
    global.time.frame_rate = frame_rate;
    global.time.frame_delay = 1000.f / frame_rate;
}

// Appelée à chaque frame
void time_update(void)
{
    global.time.now = (float)SDL_GetTicks();
    global.time.delta = (global.time.now - global.time.last) / 1000.f;
    global.time.last = global.time.now;
    ++global.time.frame_count;

    if (global.time.now - global.time.frame_last >= 1000.f)
    {
        global.time.frame_rate = global.time.frame_count;
        global.time.frame_count = 0;
        global.time.frame_last = global.time.now;
    }
}
// Appellée à la fin de chaque frame
void time_update_late(void)
{
    global.time.frame_rate = (float)SDL_GetTicks() - global.time.now;
    if (global.time.frame_delay > global.time.frame_time)
    {
        SDL_Delay(global.time.frame_delay - global.time.frame_time);
    }
}