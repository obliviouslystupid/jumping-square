#include "game.h"
#include "game_initialization.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

int main(int argc, char** argv) {
    bool audio_active = true;
    SDL_Window* windowptr = NULL;
    SDL_Renderer* rendererptr = NULL;
    TTF_Font* l_fontptr = NULL;
    TTF_Font* s_fontptr = NULL;
    if(initialize_game(&audio_active, &windowptr, &rendererptr, &l_fontptr, &s_fontptr)) {
        run_game(audio_active, rendererptr, l_fontptr, s_fontptr);
    }
    if(windowptr != NULL) {
        SDL_DestroyWindow(windowptr);
    }
    if(rendererptr != NULL) {
        SDL_DestroyRenderer(rendererptr);
    }
    if(l_fontptr != NULL) {
        TTF_CloseFont(l_fontptr);
    }
    if(s_fontptr != NULL) {
        TTF_CloseFont(s_fontptr);
    }
    SDL_Quit();
    TTF_Quit();
    return 0;
}
