#include "game_graphics.h"
#include "game_objects.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

void render_menu(SDL_Renderer* renderer, struct menu* menu, const float title_y, Uint8 menu_number, bool in_mod) {
    if(menu_number == MAIN_MENU || menu_number == LEVEL_SELECT_MENU) {
        SDL_SetRenderDrawColor(renderer, BACKGROUND_RED, BACKGROUND_GREEN, BACKGROUND_BLUE, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
    }
    if(menu->text->text != NULL) {
        TTF_SetTextColor(menu->text->text, 80, 160, 160, SDL_ALPHA_OPAQUE);
        TTF_DrawRendererText(menu->text->text, menu->text->x, title_y);
        TTF_SetTextColor(menu->text->text, 100, 200, 200, SDL_ALPHA_OPAQUE);
        TTF_DrawRendererText(menu->text->text, menu->text->x + 5, title_y + 5);
    }
    for(int i = 0; i < menu->button_count; i++) {
        if(!in_mod) {
            if(menu->buttons[i].selected) {
                SDL_SetRenderDrawColor(renderer, BUTTON_SELECTED_RED, BUTTON_SELECTED_GREEN, BUTTON_SELECTED_BLUE, SDL_ALPHA_OPAQUE);
            }
            else {
                SDL_SetRenderDrawColor(renderer, BUTTON_RED, BUTTON_GREEN, BUTTON_BLUE, SDL_ALPHA_OPAQUE);
            }
        }
        else {
            if(menu->buttons[i].selected) {
                SDL_SetRenderDrawColor(renderer, MOD_BUTTON_SELECTED_RED, MOD_BUTTON_SELECTED_GREEN, MOD_BUTTON_SELECTED_BLUE, SDL_ALPHA_OPAQUE);
            }
            else {
                SDL_SetRenderDrawColor(renderer, MOD_BUTTON_RED, MOD_BUTTON_GREEN, MOD_BUTTON_BLUE, SDL_ALPHA_OPAQUE);
            }
        }
        SDL_RenderFillRect(renderer, &menu->buttons[i].rect);
        if(menu->buttons[i].textptr != NULL) {
            TTF_DrawRendererText(menu->buttons[i].textptr, menu->buttons[i].rect.x + (menu->buttons[i].rect.w / 2) - menu->buttons[i].text_width / 2, menu->buttons[i].rect.y);
        }
    }
}
void render_frame(SDL_Renderer* renderer, union object* objects, Uint8 object_count, TTF_Text* score, const int x_offset) {
    SDL_SetRenderDrawColor(renderer, BACKGROUND_RED, BACKGROUND_GREEN, BACKGROUND_BLUE, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    for(int i = 1; i < object_count; i++) {
        switch((*(objects + i)).type) {
            case WALL:
                SDL_SetRenderDrawColor(renderer, PLATFORM_RED, PLATFORM_GREEN, PLATFORM_BLUE, SDL_ALPHA_OPAQUE);
                SDL_RenderFillRect(renderer, &(objects + i)->wall.rect);
                break;
            case PLATFORM:
                SDL_SetRenderDrawColor(renderer, PLATFORM_RED, PLATFORM_GREEN, PLATFORM_BLUE, SDL_ALPHA_OPAQUE);
                SDL_RenderFillRect(renderer, &(objects + i)->platform.rect);
                break;
            case TRAP:
                SDL_SetRenderDrawColor(renderer, TRAP_STICK_RED, TRAP_STICK_GREEN, TRAP_STICK_BLUE, SDL_ALPHA_OPAQUE);
                SDL_RenderFillRect(renderer, &(objects + i)->trap.stick_rect);
                SDL_RenderGeometry(renderer, NULL, (objects + i)->trap.spike_vertices, 3, NULL, 0);
                break;
            case BARRIER:
                SDL_SetRenderDrawColor(renderer, PLATFORM_RED, PLATFORM_GREEN, PLATFORM_BLUE, SDL_ALPHA_OPAQUE);
                SDL_RenderFillRect(renderer, &(objects + i)->barrier.wall_rect);
                SDL_RenderGeometry(renderer, NULL, (objects + i)->barrier.top_spike_vertices, 3, NULL, 0);
                SDL_RenderGeometry(renderer, NULL, (objects + i)->barrier.bottom_spike_vertices, 3, NULL, 0);
                break;
            case ENEMY:
                SDL_RenderGeometry(renderer, NULL, (objects + i)->enemy.vertices, 12, NULL, 0);
                break;
            case EXIT:
                SDL_RenderGeometry(renderer, NULL, (objects + i)->exit.vertices, 12, NULL, 0);
                break;
            case HINT:
                TTF_SetTextColor((objects + i)->hint.text, 80, 160, 160, SDL_ALPHA_OPAQUE);
                TTF_DrawRendererText((objects + i)->hint.text, (objects + i)->hint.x, (objects + i)->hint.y);
                break;
        }
    }
    SDL_SetRenderDrawColor(renderer, PLAYER_RED, PLAYER_GREEN, PLAYER_BLUE, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &objects->player.rect);
    if(score != NULL) {
        TTF_SetTextColor(score, 100, 200, 200, SDL_ALPHA_OPAQUE);
        TTF_DrawRendererText(score, VIRTUAL_SCREEN_WIDTH / 2 - x_offset, 0);
    }
}
void render_transition(SDL_Renderer* renderer, SDL_FRect* transition_rect) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
    SDL_RenderFillRect(renderer, transition_rect);
}