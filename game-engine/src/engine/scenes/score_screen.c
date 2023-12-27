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

Score *localscores;
Score *onlinescores;

size_t local_scores_count;
size_t online_scores_count;

CurlRequestScoresData *curlData;

void score_init()
{
    init_image(&menuImage, "assets/menu/Scores.png");
}

void score_reset()
{
    free(localscores);
    free(onlinescores);
    free(curlData);
    localscores = GetLocalScores(&local_scores_count);

    if (localscores != NULL)
    {
        for (size_t i = 0; i < local_scores_count; i++)
        {
            char text[50];
            snprintf(text, sizeof(text), "%s: %d", localscores[i].nom, localscores[i].score);

            printf("%s\n", text);
        }
    }

    onlinescores = malloc(6 * sizeof(Score));
    onlinescores[0].score = 0;

    if (onlinescores == NULL)
        ERROR_EXIT("Erreur d'allocation de mémoire pour onlinescores\n");

    // Créer une structure pour stocker les données de la requête
    curlData = malloc(sizeof(CurlRequestData));

    curlData->onlinescores = onlinescores;

    // Créer un thread pour effectuer la requête CURL
    HANDLE thread = CreateThread(NULL, 0, async_get_online_scores, curlData, 0, NULL);

    // Fermer le handle du thread pour libérer ses ressources
    CloseHandle(thread);
}

void display_score(SDL_Window *window)
{
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
                global.current_screen = MENU_SCREEN;
            }
            break;
        default:
            break;
        }
    }

    render_textures(score_screen_texture_slots);

    if (localscores != NULL)
    {
        for (size_t i = 0; i < local_scores_count; i++)
        {
            char text[50];
            snprintf(text, sizeof(text), "%s   %d", localscores[i].nom, localscores[i].score);

            render_text(text, 50, render_height * (0.6 - i * 0.1), WHITE, 0);
        }
    }
    else
    {
        render_text("Pas de score local ", 50, render_height * (0.5), WHITE, 0);
    }

    if (onlinescores != NULL && onlinescores[0].score != 0)
    {
        for (size_t i = 0; i < 6; i++)
        {
            char text[50];
            snprintf(text, sizeof(text), "%s   %d", onlinescores[i].nom, onlinescores[i].score);

            render_text(text, render_width / 2 + 50, render_height * (0.60 - i * 0.1), WHITE, 0);
        }
    }
    else
    {
        render_text("Pas de connexion", render_width / 2 + 50, render_height * (0.5), WHITE, 0);
    }

    render_end(window);
}

void score_screen_end()
{
    free(localscores);
    free(onlinescores);
    free(curlData);
}
