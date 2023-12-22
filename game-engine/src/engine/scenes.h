#pragma once
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "types.h"

void scenes_init();
void menu_init();
void score_init();
void display_menu(SDL_Window *window);
void display_score(SDL_Window *window);
void display_game_over(SDL_Window *window);
void show_game_over(u32 score, u8 ennemy);
