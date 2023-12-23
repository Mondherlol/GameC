#include <stdio.h>
#include <SDL2/SDL.h>

#include "../scenes.h"
#include "../render.h"
#include "../util.h"
#include "../global.h"
#include "../my_curl.h"
#include "../entity.h"

float debug_pos_x = 50;
float debug_pos_y = 50;

#define GAME_OVER_MENU_ITEMS_COUNT 2
#define ENNEMY_COUNT 5

float width;
float height;

Image gameOverImages[GAME_OVER_MENU_ITEMS_COUNT];
Image gameOverNewScoreImages[GAME_OVER_MENU_ITEMS_COUNT];
Image ennemiesImages[ENNEMY_COUNT];
int currentButtonSelection = 0;
int highScore = 3;

u8 game_over_texture_slots[16] = {0};
bool isNewHighScore = false;
u8 ennemyKiller = 0;
char playerKiller[] = "Mondher";
static char scoreText[20];
static char highScoreText[20];

// Endpoint et données POST
const char *endpoint = "/scores/test/10000";
const char *post_data = ""; // Remplacez ceci par les données POST réelles si nécessaire

void game_over_init()
{
    width = global.window_width / render_get_scale();
    height = global.window_height / render_get_scale();

    init_image(&gameOverImages[0], "assets/menu/game_over_normal_selected_replay.png");
    init_image(&gameOverImages[1], "assets/menu/game_over_normal_selected_quit.png");

    init_image(&gameOverNewScoreImages[0], "assets/menu/game_over_new_score_selected_replay.png");
    init_image(&gameOverNewScoreImages[1], "assets/menu/game_over_new_score_selected_quit.png");

    init_image(&ennemiesImages[ENTITY_ENEMY_TYPE_SMALL], "assets/spritesheets/chicken_solo.png");
    init_image(&ennemiesImages[ENTITY_ENEMY_TYPE_MEDIUM], "assets/spritesheets/bunny_solo.png");
    init_image(&ennemiesImages[ENTITY_ENEMY_TYPE_LARGE], "assets/spritesheets/rhino_solo.png");
    init_image(&ennemiesImages[ENTITY_ENEMY_TYPE_FLYING], "assets/spritesheets/bat_solo.png");
    init_image(&ennemiesImages[ENTITY_FIRE], "assets/spritesheets/fire_solo.png");
}

// Fonction de callback pour traiter la réponse de la requête POST
void handle_post_response(const char *response)
{
    printf("Réponse de la requête POST : %s\n", response);
}

void save_high_score(int score)
{
    printf("Sauvegarde du high score dans un tread dédié ...\n");
    // Construire l'URL avec le nom d'utilisateur et le score
    char endpoint[256];
    snprintf(endpoint, sizeof(endpoint), "/scores/%s/%d", global.username, score);

    // Appeler mycurl_post_async avec le nouvel endpoint
    if (mycurl_post_async(&global.curl_handle, endpoint, post_data, handle_post_response) != 0)
    {
        fprintf(stderr, "Erreur lors de l'appel de mycurl_post_async\n");
    }
}

void show_game_over(int score, u8 ennemy)
{
    // highScore = 3; // Initialisation bidon, à remplacer avec vrai highscore
    isNewHighScore = false;

    sprintf(scoreText, "%d", score);
    if (score > highScore)
    {
        isNewHighScore = true;
        highScore = score;
        save_high_score(score);
    }
    sprintf(highScoreText, "%d", highScore);
    ennemyKiller = ennemy;
    currentButtonSelection = 0;
    global.current_screen = GAME_OVER_SCREEN;
}

void display_game_over(SDL_Window *window)
{
    // initiation du rendu
    render_begin();

    // mise a jour de l'entree de lutilisateur
    // input_update();

    render_image(isNewHighScore ? &gameOverNewScoreImages[currentButtonSelection] : &gameOverImages[currentButtonSelection],
                 (vec2){0, 0},                                                                                                                                  // position
                 (vec2){gameOverImages[currentButtonSelection].width / render_get_scale(), gameOverImages[currentButtonSelection].height / render_get_scale()}, // taille
                 game_over_texture_slots);

    SDL_Event menuEvent;

    while (SDL_PollEvent(&menuEvent))
    {
        switch (menuEvent.type)
        {
        case SDL_KEYDOWN:
            switch (menuEvent.key.keysym.sym)
            {

            case SDLK_UP:
                // Changer la sélection vers l'image précédente
                currentButtonSelection = (currentButtonSelection - 1 + GAME_OVER_MENU_ITEMS_COUNT) % GAME_OVER_MENU_ITEMS_COUNT;
                break;
            case SDLK_DOWN:
                currentButtonSelection = (currentButtonSelection + 1) % GAME_OVER_MENU_ITEMS_COUNT;
                break;
            case SDLK_RETURN:
                // Gérer la sélection en fonction de currentButtonSelection
                switch (currentButtonSelection)
                {
                case 0:
                    global.current_screen = GAME_SCREEN;
                    break;
                case 1:
                    global.current_screen = MENU_SCREEN;
                    break;
                }
                break;
            case SDLK_r:
                printf("Test de la route /scores avec une requête POST dans un thread dédié.......\n");
                // Appel de la fonction mycurl_post_async
                if (mycurl_post_async(&global.curl_handle, endpoint, post_data, handle_post_response) != 0)
                {
                    fprintf(stderr, "Erreur lors de l'appel de mycurl_post_async\n");
                }
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    render_image(&ennemiesImages[ennemyKiller],
                 (vec2){529, 95},                                                                 // position
                 (vec2){ennemiesImages[ennemyKiller].width, ennemiesImages[ennemyKiller].height}, // taille
                 game_over_texture_slots);

    render_textures(game_over_texture_slots);

    // Cause de la mort
    if (ennemyKiller == ENTITY_FIRE)
        render_text("Suicide", 540, 77, RED, 1);

    // Score actuel
    render_text(scoreText, width / 2, height * 0.60, WHITE, 1);

    // Meilleur score
    render_text(highScoreText, 50, height * 0.36, YELLOW, 1);
    render_text(playerKiller, 75, height * 0.31, WHITE, 1);

    render_end(window);
}
