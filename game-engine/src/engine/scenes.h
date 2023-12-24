#pragma once
#include <SDL2/SDL.h>

void scenes_init();
void menu_init();
void score_init();
void score_reset();
void display_menu(SDL_Window *window);
void display_score(SDL_Window *window);
