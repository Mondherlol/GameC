#pragma once
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "types.h"

void scenes_init();
void scenes_end();
void menu_init();
void score_init();
void score_reset();
void username_menu_init();
void display_menu(SDL_Window *window);
void display_score(SDL_Window *window);
void display_game_over(SDL_Window *window);
void show_game_over(int score, u8 ennemy, const char *nameKiller);
void display_username_menu(SDL_Window *window, u32 texture_slots[32]);
void reset_menu();

void score_screen_end();
