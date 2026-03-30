#ifndef GAME_GRAPHICS_H
#define GAME_GRAPHICS_H
#include "game_objects.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#define VIRTUAL_SCREEN_WIDTH 1920
#define VIRTUAL_SCREEN_HEIGHT 1080
#define MAIN_MENU 0
#define LEVEL_SELECT_MENU 1
#define PAUSE_MENU 2
#define DEATH_MENU 3

#define BACKGROUND_RED 252
#define BACKGROUND_GREEN 246
#define BACKGROUND_BLUE 230

#define BUTTON_RED 100
#define BUTTON_GREEN 200
#define BUTTON_BLUE 200

#define BUTTON_SELECTED_RED 80
#define BUTTON_SELECTED_GREEN 160
#define BUTTON_SELECTED_BLUE 160

#define MOD_BUTTON_RED 100
#define MOD_BUTTON_GREEN 200
#define MOD_BUTTON_BLUE 100

#define MOD_BUTTON_SELECTED_RED 80
#define MOD_BUTTON_SELECTED_GREEN 160
#define MOD_BUTTON_SELECTED_BLUE 80

#define PLAYER_RED 100
#define PLAYER_GREEN 0
#define PLAYER_BLUE 0

#define PLATFORM_RED 155
#define PLATFORM_GREEN 199
#define PLATFORM_BLUE 204

#define TRAP_STICK_RED 135
#define TRAP_STICK_GREEN 95
#define TRAP_STICK_BLUE 58

struct menu {
    struct menu_text* text;
    struct button* buttons;
    Uint8 button_count;
};

struct menu_text {
    TTF_Text* text;
    int x;
};

struct button {
    TTF_Text* textptr;
    SDL_FRect rect;
    int text_width;
    bool selected;
};

void render_menu(SDL_Renderer* renderer, struct menu* menu, const float title_y, Uint8 menu_number, bool in_mod);
void render_frame(SDL_Renderer* renderer, union object* objects, Uint8 object_count, TTF_Text* score, const int x_offset);
void render_transition(SDL_Renderer* renderer, SDL_FRect* transition_rect);

#endif