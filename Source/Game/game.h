#ifndef GAME_H
#define GAME_H
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

void run_game(bool audio_active, SDL_Renderer* renderer, TTF_Font* l_font, TTF_Font* s_font);

#endif