#ifndef GAME_INITIALIZATION_H
#define GAME_INITIALIZATION_H
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

bool initialize_game(bool* audio, SDL_Window** window, SDL_Renderer** renderer, TTF_Font** l_font, TTF_Font** s_font);

#endif