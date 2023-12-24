#include <stdio.h>
#include <SDL2/SDL.h>
#include "../scenes.h"
#include "../render.h"
#include "../util.h"
#include "../global.h"
#include "../my_curl.h"
#include "../scores.h"

Image menuImage;

u8 score_screen_texture_slots[16] = {0};


//Pour stocker les struc scores, declaration d'un pointeur 
Score *localscores;
Score *onlinescores;

//Pour stocker le nombre de scores
size_t count;
size_t online_count;

void score_init()
{
    init_image(&menuImage, "assets/menu/Scores.png");
    local_score_init();
    // WriteLocalScore("sarra",362505);
}

void score_reset(){
    localscores = GetLocalScores(&count);
        
    if (localscores != NULL)
    {
        for (size_t i = 0; i < count; i++)
        {
            char text[50];
            snprintf(text, sizeof(text), "%s: %d", localscores[i].nom, localscores[i].score);
           
            printf("%s\n", text);  
        }

      
        free(localscores);
    }


    printf("Test de la route /scores avec une requête GET dans un thread dédié.......\n");
    // Créer une structure pour stocker les données de la requête
    CurlRequestData *curlData = malloc(sizeof(CurlRequestData));
    curlData->handle = &global.curl_handle;
    curlData->endpoint = "/scores?limit=8";
    // Créer un thread pour effectuer la requête
    HANDLE thread = CreateThread(NULL, 0, async_curl_request, curlData, 0, NULL);
    CloseHandle(thread); // Fermer le handle du thread pour libérer ses ressources lorsqu'il a terminé

}


void display_score(SDL_Window *window)
{

    // initiation du rendu
    render_begin();

    render_image(&menuImage,
                 (vec2){0, 0},                                                                        // position
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

    render_textures(score_screen_texture_slots);

   // afficher le contenu du tab 
    for (size_t i = 0; i < count; i++)
    {
        char text[50];
        snprintf(text, sizeof(text), "%s: %d", localscores[i].nom, localscores[i].score);
        
        render_text(text, 175, render_height * (0.4 + i * 0.1), WHITE, 1);
    }

    render_end(window);
}

