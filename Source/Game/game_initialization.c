#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

bool initialize_game(bool* audio, SDL_Window** window, SDL_Renderer** renderer, TTF_Font** l_font, TTF_Font** s_font) {
    if(!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Video Initialization Failed!", "The video subsystem has failed to initialize.", NULL);
        return false;
    }
    if(!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Audio Initialization Failed!", "The audio subsystem has failed to initialize.", NULL);
        *audio = false;
    }
    SDL_Rect display = {0, 0, 2, 2}; //2s under the assumption that most moniters are greater than 1x1
    SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &display);
    if(!SDL_CreateWindowAndRenderer("Game", display.w / 2, display.h / 2, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED, window, renderer)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Window Creation Failed!", "The window has failed to be created.", NULL);
        return false;
    }
    if(!TTF_Init()) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Font Library Initialization Failed!", "The font library has failed to initialize.", NULL);
    }
    else {
        *l_font = TTF_OpenFont("./Fonts/AutourOne-Regular.ttf", 128.0f);
        *s_font = TTF_OpenFont("./Fonts/AutourOne-Regular.ttf", 32.0f);
        if(*l_font == NULL || *s_font == NULL) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Font Loading Failed!", "One or more fonts have failed to load.", NULL);
        }
    }
    return true;
}