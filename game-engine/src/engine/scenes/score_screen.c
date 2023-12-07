#include <stdio.h>
#include <SDL2/SDL.h>
#include "../scenes.h"
#include "../render.h"
#include "../util.h"
#include "../global.h"
#include "../my_curl.h"


Image menuImage;

u8 score_screen_texture_slots[8] = {0};


void score_init()
{

    init_image(&menuImage, "assets/Mondher/Scores.png");
  
 
}

void display_score(SDL_Window *window)
{

    // initiation du rendu
    render_begin();

    // render_text("Rendu avant l'image", width / 2, height * 0.8, RED, 1);

    render_image(&menuImage,
                 (vec2){0, 0},                                                                                                              // position
                 (vec2){menuImage.width / render_get_scale(), menuImage.height / render_get_scale()}, // taille
                 score_screen_texture_slots);





    SDL_Event menuEvent;

    
while (SDL_PollEvent(&menuEvent))
{
    switch (menuEvent.type)
    {
        case SDL_KEYDOWN:
            if (menuEvent.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
            {
                // Code à exécuter lorsque la touche Échap est enfoncée
                global.current_screen = MENU_SCREEN;
            }
            break;
        default:
            break;
    }
}
    

    render_end(window, score_screen_texture_slots);

    //SDL_GL_SwapWindow(window); // Mettre à jour la fenêtre avec le rendu OPENGL
}
