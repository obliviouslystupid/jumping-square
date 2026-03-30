#include "game_audio.h"
#include "game_graphics.h"
#include "game_objects.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>

#define LEVEL_COUNT 21
#define PLAY_BUTTON 0
#define ENDLESS_BUTTON 1
#define EXIT_BUTTON 2
#define RESET_BUTTON 0
#define MENU_BUTTON 1
#define BACK_BUTTON 0
#define MOD_BUTTON 1

void check_for_collisions_preemptively(union object* objects, Uint8 object_count);
bool check_for_vertical_collision_preemptively(struct player* player, SDL_FRect* hitbox);
bool check_for_horizontal_collision_preemptively(struct player* player, SDL_FRect* hitbox);
bool check_for_current_collision(struct player* player, SDL_FRect* hitbox);
void reset_score_label(TTF_Text** score_label, TTF_Font* font, TTF_TextEngine* text_engine, Uint32* score);
void run_game(bool audio_active, SDL_Renderer* renderer, TTF_Font* l_font, TTF_Font* s_font) {
    struct audio* theme_audio = NULL;
    struct audio* jump_audio = NULL;
    SDL_AudioDeviceID device_id = 0;
    if(audio_active) {
        device_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
        theme_audio = create_audio("./Audio/titlescreen.wav", device_id);
        jump_audio = create_audio("./Audio/jump.wav", device_id);
        play_audio(theme_audio);
    }
    bool running = true;
    SDL_Event event;
    Uint64 starting_time;
    const Uint32 frame_length_ms = 17;
    TTF_TextEngine* text_engineptr = (l_font != NULL) ? TTF_CreateRendererTextEngine(renderer) : NULL;
    TTF_Text* titleptr = (l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, "JUMPING SQUARE", 0) : NULL;
    TTF_Text* pauseptr = (l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, "PAUSED", 0) : NULL;
    TTF_Text* deathptr = (l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, "YOU DIED", 0) : NULL;
    TTF_Text* level_selectptr = (l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, "SELECT A LEVEL", 0) : NULL;
    struct menu_text title_menu_text = {titleptr, 0};
    struct menu_text pause_menu_text = {pauseptr, 0};
    struct menu_text death_menu_text = {deathptr, 0};
    struct menu_text level_select_menu_text = {level_selectptr, 0};
    float title_y = 100;
    bool title_lowering = true;
    struct button main_menu_buttons[] = {{(l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, "PLAY", 0) : NULL, {VIRTUAL_SCREEN_WIDTH / 2 - VIRTUAL_SCREEN_WIDTH / 8, VIRTUAL_SCREEN_HEIGHT / 2 - 100, VIRTUAL_SCREEN_WIDTH / 4, 180}, 0, false}, {(l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, "∞", 0) : NULL, {VIRTUAL_SCREEN_WIDTH / 2 - VIRTUAL_SCREEN_WIDTH / 8, VIRTUAL_SCREEN_HEIGHT / 2 + 100, VIRTUAL_SCREEN_WIDTH / 4, 180}, 0, false}, {(l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, "EXIT", 0) : NULL, {VIRTUAL_SCREEN_WIDTH / 2 - VIRTUAL_SCREEN_WIDTH / 8, VIRTUAL_SCREEN_HEIGHT / 2 + 300, VIRTUAL_SCREEN_WIDTH / 4, 180}, 0, false}};
    struct button pause_menu_buttons[] = {{(l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, "RESET", 0) : NULL, {VIRTUAL_SCREEN_WIDTH / 2 - VIRTUAL_SCREEN_WIDTH / 8, VIRTUAL_SCREEN_HEIGHT / 2 - 100, VIRTUAL_SCREEN_WIDTH / 4, 180}, 0, false}, {(l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, "MENU", 0) : NULL, {VIRTUAL_SCREEN_WIDTH / 2 - VIRTUAL_SCREEN_WIDTH / 8, VIRTUAL_SCREEN_HEIGHT / 2 + 100, VIRTUAL_SCREEN_WIDTH / 4, 180}, 0, false}};
    struct button level_select_menu_buttons[23]; 
    struct menu menus[] = {{&title_menu_text, main_menu_buttons, sizeof(main_menu_buttons) / sizeof(main_menu_buttons[0])}, {&level_select_menu_text, level_select_menu_buttons, sizeof(level_select_menu_buttons) / sizeof(level_select_menu_buttons[0])}, {&pause_menu_text, pause_menu_buttons, sizeof(pause_menu_buttons) / sizeof(pause_menu_buttons[0])}, {&death_menu_text, pause_menu_buttons, sizeof(pause_menu_buttons) / sizeof(pause_menu_buttons[0])}};
    float level_button_w = 180;
    float level_button_h = 180;
    float level_button_gap = 18;
    float level_button_x = (VIRTUAL_SCREEN_WIDTH / 2) - ((7.0 / 2) * (level_button_w + level_button_gap));
    float level_button_y = VIRTUAL_SCREEN_HEIGHT / 2 - 100;
    char score_string[11];
    level_select_menu_buttons[0].textptr = (l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, "<", 0) : NULL;
    level_select_menu_buttons[0].rect.x = level_button_gap;
    level_select_menu_buttons[0].rect.y = VIRTUAL_SCREEN_HEIGHT - level_button_h - level_button_gap;
    level_select_menu_buttons[0].rect.w = level_button_w;
    level_select_menu_buttons[0].rect.h = level_button_h;
    level_select_menu_buttons[0].selected = false;
    level_select_menu_buttons[1].textptr = (s_font != NULL) ? TTF_CreateText(text_engineptr, s_font, "Mod", 0) : NULL;
    level_select_menu_buttons[1].rect.x = level_button_gap;
    level_select_menu_buttons[1].rect.y = VIRTUAL_SCREEN_HEIGHT - level_button_h * 2 - level_button_gap * 2;
    level_select_menu_buttons[1].rect.w = level_button_w;
    level_select_menu_buttons[1].rect.h = level_button_h;
    level_select_menu_buttons[1].selected = false;
    for(int i = 2; i <= LEVEL_COUNT + 1; i++) {
        sprintf(score_string, "%u", i - 1);
        level_select_menu_buttons[i].textptr = (l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, score_string, 0) : NULL;
        level_select_menu_buttons[i].rect.x = level_button_x;
        level_select_menu_buttons[i].rect.y = level_button_y;
        level_select_menu_buttons[i].rect.w = level_button_w;
        level_select_menu_buttons[i].rect.h = level_button_h;
        level_select_menu_buttons[i].selected = false;
        level_button_x += level_button_w + level_button_gap;
        if(level_button_x == (VIRTUAL_SCREEN_WIDTH / 2) - ((7.0 / 2) * (level_button_w + level_button_gap)) + (7 * (level_button_w + level_button_gap))) {
            level_button_x = (VIRTUAL_SCREEN_WIDTH / 2) - ((7.0 / 2) * (level_button_w + level_button_gap));
            level_button_y += level_button_h + level_button_gap;
        }
    }
    for(int i = 0; i < sizeof(menus) / sizeof(menus[0]); i++) {
        TTF_GetTextSize(menus[i].text->text, &menus[i].text->x, NULL);
        menus[i].text->x = VIRTUAL_SCREEN_WIDTH / 2 - menus[i].text->x / 2;
        for(int j = 0; j < menus[i].button_count; j++) {
            TTF_GetTextSize(menus[i].buttons[j].textptr, &menus[i].buttons[j].text_width, NULL);
        }
    }
    int held_button = -1;
    bool in_mod = false;
    bool in_menu = true;
    bool paused = false;
    bool can_escape = true;
    bool in_main = true;
    bool endless = false;
    Uint32 triple_jump_odds = 0;
    Uint8 current_menu = MAIN_MENU;
    SDL_rand(1); // the first SDL_rand call is not very random for some reason so this avoids the lowest platform from being in the same place as a previous instance of the game
    union object* level_objects = NULL;
    Uint8 object_count;
    Uint8 current_level = 0;
    bool finished_level = false;
    Uint8 end_level_frames = 0;
    SDL_FRect end_rect = {0, 0, 0, 0};
    Sint32 exit_y = 0;
    int real_screen_width = 0;
    int real_screen_height = 0;
    float screen_scale_x = 0;
    float screen_scale_y = 0;
    Uint32 score = 0;
    TTF_Text* score_labelptr = (l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, "0", 0) : NULL;
    int zero_width = 0;
    if(l_font != NULL) {
        TTF_GetTextSize(score_labelptr, &zero_width, NULL);
    }
    SDL_GetWindowSize(SDL_GetRenderWindow(renderer), &real_screen_width, &real_screen_height);
    screen_scale_x = (float)real_screen_width / VIRTUAL_SCREEN_WIDTH;
    screen_scale_y = (float)real_screen_height / VIRTUAL_SCREEN_HEIGHT;
    SDL_SetRenderScale(renderer, screen_scale_x, screen_scale_y);
    while(running) {
        starting_time = SDL_GetTicks();
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_EVENT_QUIT:
                    //this causes the game to process one more frame before it closes
                    running = false;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    SDL_GetWindowSize(SDL_GetWindowFromID(event.window.windowID), &real_screen_width, &real_screen_height);
                    screen_scale_x = (float)real_screen_width / VIRTUAL_SCREEN_WIDTH;
                    screen_scale_y = (float)real_screen_height / VIRTUAL_SCREEN_HEIGHT;
                    SDL_SetRenderScale(renderer, screen_scale_x, screen_scale_y);
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    if(!in_menu) {
                        break;
                    }
                    for(int i = 0; i < menus[current_menu].button_count; i++) {
                        if(event.motion.x >= (menus[current_menu].buttons[i].rect.x * screen_scale_x) && event.motion.x <= ((menus[current_menu].buttons[i].rect.x + menus[current_menu].buttons[i].rect.w) * screen_scale_x) && event.motion.y >= (menus[current_menu].buttons[i].rect.y * screen_scale_y) && event.motion.y <= ((menus[current_menu].buttons[i].rect.y + menus[current_menu].buttons[i].rect.h) * screen_scale_y)) {
                            menus[current_menu].buttons[i].selected = true;
                        }
                        else {
                            menus[current_menu].buttons[i].selected = false;
                        }
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if(!in_menu) {
                        break;
                    }
                    for(int i = 0; i < menus[current_menu].button_count; i++) {
                        if(menus[current_menu].buttons[i].selected) {
                            held_button = i;
                            break;
                        }
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if(!in_menu) {
                        break;
                    }
                    if(menus[current_menu].buttons[held_button].selected) {
                        switch(current_menu) {
                            case MAIN_MENU:
                                switch(held_button) {
                                    case PLAY_BUTTON:
                                        current_menu = LEVEL_SELECT_MENU;
                                        break;
                                    case ENDLESS_BUTTON:
                                        in_menu = false;
                                        in_main = false;
                                        can_escape = true;
                                        endless = true;
                                        current_menu = PAUSE_MENU;
                                        current_level = 0;
                                        load_objects(&level_objects, current_level, &object_count, text_engineptr, s_font, false);
                                        destroy_audio(theme_audio);
                                        theme_audio = create_audio("./Audio/early.wav", device_id);
                                        play_audio(theme_audio);
                                        break;
                                    case EXIT_BUTTON:
                                        running = false;
                                        break;
                                }
                                break;
                            case PAUSE_MENU:
                            case DEATH_MENU:
                                switch(held_button) {
                                    case RESET_BUTTON:
                                        if(!load_objects(&level_objects, current_level, &object_count, text_engineptr, s_font, in_mod)) {
                                            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Level loading failed!", "The level has failed to load.", NULL);
                                            running = false;
                                            break;
                                        }
                                        reset_score_label(&score_labelptr, l_font, text_engineptr, &score);
                                        in_menu = false;
                                        paused = false;
                                        break;
                                    case MENU_BUTTON:
                                        if(audio_active) {
                                            destroy_audio(theme_audio);
                                            theme_audio = create_audio("./Audio/titlescreen.wav", device_id);
                                            play_audio(theme_audio);
                                        }
                                        reset_score_label(&score_labelptr, l_font, text_engineptr, &score);
                                        in_mod = false;
                                        in_menu = true;
                                        paused = false;
                                        can_escape = false;
                                        in_main = true;
                                        endless = false;
                                        current_menu = MAIN_MENU;
                                        for(int i = 0; i < menus[current_menu].button_count; i++) {
                                            menus[current_menu].buttons[i].selected = false;
                                        }
                                        break;
                                    break;
                                }
                                break;
                            case LEVEL_SELECT_MENU:
                                switch(held_button) {
                                    case BACK_BUTTON:
                                        in_mod = false;
                                        current_menu = MAIN_MENU;
                                        break;
                                    case MOD_BUTTON:
                                        in_mod = !in_mod;
                                        break;
                                    default:
                                        if(!load_objects(&level_objects, held_button - 1, &object_count, text_engineptr, s_font, in_mod)) {
                                            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Level loading failed!", "The level has failed to load.", NULL);
                                            break;
                                        }
                                        destroy_audio(theme_audio);
                                        if(held_button - 1 >= 1 && held_button - 1 <= 7) {
                                            theme_audio = create_audio("./Audio/early.wav", device_id);
                                        }
                                        else if(held_button - 1 >= 8 && held_button - 1 <= 20) {
                                            theme_audio = create_audio("./Audio/rows.wav", device_id);
                                        }
                                        else {
                                            theme_audio = create_audio("./Audio/boss.wav", device_id);
                                        }
                                        play_audio(theme_audio);
                                        current_level = held_button - 1;
                                        in_menu = false;
                                        in_main = false;
                                        can_escape = true;
                                        current_menu = PAUSE_MENU;
                                        finished_level = false;
                                        end_level_frames = 0;
                                        break;
                                }
                        }
                    }
                    break;
                case SDL_EVENT_KEY_DOWN:
                    switch(event.key.scancode) {
                        case SDL_SCANCODE_UP:
                        case SDL_SCANCODE_W:
                            if(!in_menu) {
                                if((*level_objects).player.can_jump && (*level_objects).player.jump_count > 0) {
                                    (*level_objects).player.vertical_movement = -10;
                                    (*level_objects).player.can_jump = false;
                                    (*level_objects).player.jump_count--;
                                    if(audio_active) {
                                        play_audio(jump_audio);
                                    }
                                }
                            }
                            break;
                        case SDL_SCANCODE_LEFT:
                        case SDL_SCANCODE_A:
                            if(!in_menu) {
                                (*level_objects).player.left_movement = 10;
                            }
                            break;
                        case SDL_SCANCODE_RIGHT:
                        case SDL_SCANCODE_D:
                            if(!in_menu) {
                                (*level_objects).player.right_movement = 10;
                            }
                            break;
                        case SDL_SCANCODE_ESCAPE:
                            if(can_escape && !in_main && (*level_objects).player.alive) {
                                in_menu = !in_menu;
                                paused = !paused;
                                current_menu = PAUSE_MENU;
                                for(int i = 0; i < menus[current_menu].button_count; i++) {
                                    menus[current_menu].buttons[i].selected = false;
                                }
                                can_escape = false;
                                title_y = 100;
                            }
                            break;
                    }
                    break;
                case SDL_EVENT_KEY_UP:
                    switch(event.key.scancode) {
                        case SDL_SCANCODE_UP:
                        case SDL_SCANCODE_W:
                            if(!in_menu && !finished_level) {
                                (*level_objects).player.can_jump = true;
                            }
                            break;
                        case SDL_SCANCODE_LEFT:
                        case SDL_SCANCODE_A:
                            if(!in_menu) {
                                (*level_objects).player.left_movement = 0;
                            }
                            break;
                        case SDL_SCANCODE_RIGHT:
                        case SDL_SCANCODE_D:
                            if(!in_menu) {
                                (*level_objects).player.right_movement = 0;
                            }
                            break;
                        case SDL_SCANCODE_ESCAPE:
                            can_escape = true;
                    }
                    break;
            }
        }
        if(audio_active) {
            loop_if_done(theme_audio);
        }
        if(!in_menu && (*level_objects).player.alive && !finished_level) {
            check_for_collisions_preemptively(level_objects, object_count);
            if(!level_objects->player.alive) goto dead;
            for(int i = 3; i < object_count; i++) {
                if((level_objects + i)->type == EXIT) {
                    exit_y = (level_objects + i)->exit.hitbox.y;
                    if((level_objects + i)->exit.collided) {
                        finished_level = true;
                        current_level = (level_objects + i)->exit.exit_level;
                        break;
                    }
                }
            }
            (*level_objects).player.hitbox.x = (*level_objects).player.rect.x += (*level_objects).player.right_movement - (*level_objects).player.left_movement;
            (*level_objects).player.hitbox.y = (*level_objects).player.rect.y += (int)(*level_objects).player.vertical_movement;
            if(!(*level_objects).player.on_ground) {
                (*level_objects).player.vertical_movement += 0.25;
            }
            if(endless) {
                move_objects(level_objects, object_count, (level_objects->player.vertical_movement < 0 && level_objects->player.hitbox.y < VIRTUAL_SCREEN_HEIGHT / 2) ? VIRTUAL_SCREEN_HEIGHT / 2 - level_objects->player.hitbox.y : 0);
            }
            else {
                move_objects(level_objects, object_count, (level_objects->player.vertical_movement < 0 && level_objects->player.hitbox.y < VIRTUAL_SCREEN_HEIGHT / 2 && exit_y <= VIRTUAL_SCREEN_HEIGHT / 2) ? VIRTUAL_SCREEN_HEIGHT / 2 - level_objects->player.hitbox.y : 0);
            }
            if(level_objects->player.vertical_movement < 0 && level_objects->player.hitbox.y < VIRTUAL_SCREEN_HEIGHT / 2 && (endless || exit_y <= VIRTUAL_SCREEN_HEIGHT / 2)) {
                (*level_objects).player.hitbox.y = (*level_objects).player.rect.y = VIRTUAL_SCREEN_HEIGHT / 2;
                score -= (*level_objects).player.vertical_movement;
            }
            if(endless) {
                float height_difference;
                triple_jump_odds = score / 2000;
                for(int i = 3; i < object_count; i++) {
                    switch((level_objects + i)->type) {
                        case PLATFORM:
                            if((level_objects + i)->platform.hitbox.y >= VIRTUAL_SCREEN_HEIGHT) {
                                (level_objects + i)->platform.hitbox.x = (level_objects + i)->platform.rect.x = SDL_rand(VIRTUAL_SCREEN_WIDTH - level_objects->player.hitbox.w * 2 - (level_objects + i)->platform.hitbox.w) + level_objects->player.hitbox.w;
                                (level_objects + i)->platform.movement_direction = (SDL_rand(triple_jump_odds) / 7 >= 1) ? 1 : 0;
                                (level_objects + i)->platform.shrinking = (SDL_rand(triple_jump_odds) / 7 >= 1) ? 1 : 0;
                                (level_objects + i)->platform.collided = false;
                                (level_objects + i)->platform.hitbox.w = (level_objects + i)->platform.rect.w = level_objects->player.hitbox.w * 3;
                                if(i == 3) {
                                    (level_objects + i)->platform.hitbox.y = (level_objects + i)->platform.rect.y = (SDL_rand(triple_jump_odds) / 5 >= 1) ? (level_objects + 6)->platform.hitbox.y - (level_objects + 6)->platform.hitbox.h * 9 : (level_objects + 6)->platform.hitbox.y - (level_objects + 6)->platform.hitbox.h * 6;
                                }
                                else {
                                    (level_objects + i)->platform.hitbox.y = (level_objects + i)->platform.rect.y = (SDL_rand(triple_jump_odds) / 5 >= 1) ? (level_objects + i - 1)->platform.hitbox.y - (level_objects + i - 1)->platform.hitbox.h * 9 : (level_objects + i - 1)->platform.hitbox.y - (level_objects + i - 1)->platform.hitbox.h * 6;
                                }
                            }
                            break;
                        case TRAP:
                            if((level_objects + i)->trap.stick_hitbox.y >= VIRTUAL_SCREEN_HEIGHT) {
                                height_difference = (SDL_rand(2) == 0) ? 2500 : 3500;
                                (level_objects + i)->trap.stick_hitbox.y = (level_objects + i)->trap.stick_rect.y = (level_objects + i)->trap.spike_hitbox.y = (level_objects + i)->trap.spike_vertices[0].position.y -= height_difference;
                                (level_objects + i)->trap.spike_vertices[1].position.y -= height_difference;
                                (level_objects + i)->trap.spike_vertices[2].position.y -= height_difference;
                                (level_objects + i)->trap.cycle = SDL_rand(UINT8_MAX);
                                (level_objects + i)->trap.stick_hitbox.w = (level_objects + i)->trap.stick_rect.w = ((level_objects + i)->trap.cycle < 128) ? (level_objects + i)->trap.cycle * 4 : (128 - (level_objects + i)->trap.cycle % 128) * 4;
                                if((level_objects + i)->trap.stick_hitbox.x == (level_objects + 1)->wall.hitbox.x + (level_objects + 1)->wall.hitbox.w) {
                                    (level_objects + i)->trap.spike_hitbox.x = (level_objects + i)->trap.stick_hitbox.x + (level_objects + i)->trap.stick_hitbox.w;
                                    (level_objects + i)->trap.spike_vertices[0].position.x = (level_objects + i)->trap.spike_vertices[1].position.x = (level_objects + i)->trap.spike_hitbox.x;
                                    (level_objects + i)->trap.spike_vertices[2].position.x = (level_objects + i)->trap.spike_hitbox.x + (level_objects + i)->trap.spike_hitbox.w;
                                }
                                else {
                                    (level_objects + i)->trap.stick_hitbox.x = (level_objects + i)->trap.stick_rect.x = (level_objects + 2)->wall.hitbox.x - (level_objects + i)->trap.stick_hitbox.w;
                                    (level_objects + i)->trap.spike_hitbox.x = (level_objects + i)->trap.stick_hitbox.x - (level_objects + i)->trap.spike_hitbox.w;
                                    (level_objects + i)->trap.spike_vertices[0].position.x = (level_objects + i)->trap.spike_vertices[1].position.x = (level_objects + i)->trap.spike_hitbox.x + (level_objects + i)->trap.spike_hitbox.w;
                                    (level_objects + i)->trap.spike_vertices[2].position.x = (level_objects + i)->trap.spike_hitbox.x;
                                }
                            }
                            break;
                        case BARRIER:
                            if((level_objects + i)->barrier.top_spike_hitbox.y >= VIRTUAL_SCREEN_HEIGHT) {
                                height_difference = (SDL_rand(2) == 0) ? 4000 : 6500;
                                (level_objects + i)->barrier.wall_hitbox.y = (level_objects + i)->barrier.wall_rect.y = (level_objects + i)->barrier.top_spike_vertices[0].position.y = (level_objects + i)->barrier.top_spike_vertices[1].position.y -= height_difference;
                                (level_objects + i)->barrier.wall_hitbox.x = (level_objects + i)->barrier.wall_rect.x = (level_objects + i)->barrier.top_spike_vertices[0].position.x = (level_objects + i)->barrier.bottom_spike_vertices[0].position.x = (level_objects + i)->barrier.top_spike_hitbox.x = (level_objects + i)->barrier.bottom_spike_hitbox.x = SDL_rand(VIRTUAL_SCREEN_WIDTH - level_objects->player.hitbox.w * 13) + level_objects->player.hitbox.w * 6;
                                (level_objects + i)->barrier.top_spike_vertices[0].position.x = (level_objects + i)->barrier.bottom_spike_vertices[0].position.x = (level_objects + i)->barrier.wall_hitbox.x;
                                (level_objects + i)->barrier.top_spike_vertices[1].position.x = (level_objects + i)->barrier.bottom_spike_vertices[1].position.x = (level_objects + i)->barrier.wall_hitbox.x + (level_objects + i)->barrier.wall_hitbox.w;
                                (level_objects + i)->barrier.top_spike_vertices[2].position.x = (level_objects + i)->barrier.bottom_spike_vertices[2].position.x = (level_objects + i)->barrier.wall_hitbox.x + (level_objects + i)->barrier.wall_hitbox.w / 2;
                                (level_objects + i)->barrier.top_spike_hitbox.y = (level_objects + i)->barrier.top_spike_vertices[2].position.y -= height_difference;
                                (level_objects + i)->barrier.bottom_spike_hitbox.y = (level_objects + i)->barrier.bottom_spike_vertices[0].position.y = (level_objects + i)->barrier.bottom_spike_vertices[1].position.y -= height_difference;
                                (level_objects + i)->barrier.bottom_spike_vertices[2].position.y -= height_difference;
                            }
                            break;
                        case ENEMY:
                            if((level_objects + i)->enemy.hitbox.y >= VIRTUAL_SCREEN_HEIGHT + level_objects->player.hitbox.h * 2) {
                                height_difference = (SDL_rand(2) == 0) ? 2000 : 3000;
                                (level_objects + i)->enemy.hitbox.y -= height_difference;
                                (level_objects + i)->enemy.hitbox.x = (level_objects + i)->enemy.vertices[0].position.x = SDL_rand(VIRTUAL_SCREEN_WIDTH - level_objects->player.hitbox.w * 2 - (level_objects + i)->enemy.hitbox.w) + level_objects->player.hitbox.w;
                                (level_objects + i)->enemy.hitbox.w += (level_objects + i)->enemy.hitbox.w / 3;
                                (level_objects + i)->enemy.hitbox.h += (level_objects + i)->enemy.hitbox.h / 3;
                                (level_objects + i)->enemy.vertices[1].position.x = (level_objects + i)->enemy.vertices[2].position.x = (level_objects + i)->enemy.vertices[6].position.x = (level_objects + i)->enemy.vertices[7].position.x = (level_objects + i)->enemy.vertices[11].position.x = (level_objects + i)->enemy.hitbox.x + (level_objects + i)->enemy.hitbox.w / 3;
                                (level_objects + i)->enemy.vertices[3].position.x = (level_objects + i)->enemy.hitbox.x + (level_objects + i)->enemy.hitbox.w;
                                (level_objects + i)->enemy.vertices[4].position.x = (level_objects + i)->enemy.vertices[5].position.x = (level_objects + i)->enemy.vertices[8].position.x = (level_objects + i)->enemy.vertices[9].position.x = (level_objects + i)->enemy.vertices[10].position.x = (level_objects + i)->enemy.hitbox.x + ((level_objects + i)->enemy.hitbox.w / 3) * 2;
                                for(int j = 0; j < 12; j++) {
                                    (level_objects + i)->enemy.vertices[j].position.y -= height_difference;
                                }
                                (level_objects + i)->enemy.hitbox.w *= 0.75;
                                (level_objects + i)->enemy.hitbox.h *= 0.75;
                                (level_objects + i)->enemy.cycle = SDL_rand(UINT8_MAX);
                            }
                            break;
                    }
                }
                sprintf(score_string, "%u", score);
                TTF_DestroyText(score_labelptr);
                score_labelptr = (l_font != NULL) ? TTF_CreateText(text_engineptr, l_font, score_string, 0) : NULL;
                render_frame(renderer, level_objects, object_count, score_labelptr, (((int)SDL_log10((score > 0) ? score : 1)+ 1) * zero_width) / 2);
            }
            else {
                render_frame(renderer, level_objects, object_count, NULL, 0);
            }
            if((*level_objects).player.hitbox.y >= VIRTUAL_SCREEN_HEIGHT) {
                dead:
                (*level_objects).player.alive = false;
                in_menu = true;
                current_menu = DEATH_MENU;
                for(int i = 0; i < menus[current_menu].button_count; i++) {
                    menus[current_menu].buttons[i].selected = false;
                }
            }
        }
        else if(finished_level) {

            render_frame(renderer, level_objects, object_count, NULL, 0);
            if(end_level_frames == 0) {
                end_rect.x = level_objects->player.hitbox.x + level_objects->player.hitbox.w / 2;
                end_rect.y = level_objects->player.hitbox.y + level_objects->player.hitbox.h / 2;
                end_rect.w = 1;
                end_rect.h = 0.5625;
            }
            else if(end_level_frames == 60) {
                load_objects(&level_objects, current_level, &object_count, text_engineptr, s_font, in_mod);
                if(current_level == 1) {
                   destroy_audio(theme_audio);
                    theme_audio = create_audio("./Audio/early.wav", device_id);
                    play_audio(theme_audio); 
                }
                if(current_level == 8) {
                    destroy_audio(theme_audio);
                    theme_audio = create_audio("./Audio/rows.wav", device_id);
                    play_audio(theme_audio);
                }
                else if(current_level == 21) {
                    destroy_audio(theme_audio);
                    theme_audio = create_audio("./Audio/boss.wav", device_id);
                    play_audio(theme_audio);
                }
                else if(current_level == 22) {
                    destroy_audio(theme_audio);
                    theme_audio = create_audio("./Audio/end.wav", device_id);
                    play_audio(theme_audio);
                }
            }
            render_transition(renderer, &end_rect);
            if(end_rect.h < VIRTUAL_SCREEN_HEIGHT * 2 && end_level_frames < 60) {
                end_rect.w *= 1.5;
                end_rect.h *= 1.5;                
            }
            else if(end_level_frames >= 60) {
                end_rect.w /= 1.5;
                end_rect.h /= 1.5;
            }
            end_rect.x = level_objects->player.hitbox.x + level_objects->player.hitbox.w / 2 - end_rect.w / 2;
            end_rect.y = level_objects->player.hitbox.y + level_objects->player.hitbox.h / 2 - end_rect.h / 2;
            end_level_frames++;
            if(end_level_frames == 90) {
                end_level_frames = 0;
                end_rect.w = end_rect.h = 0;
                finished_level = false;
            }
        }
        else {
            if(!in_main) {
                if(paused || !(*level_objects).player.alive) {
                    render_frame(renderer, level_objects, object_count, NULL, 0);
                }                
            }
            render_menu(renderer, &menus[current_menu], title_y, current_menu, in_mod);
            if(title_lowering) {
                title_y += 0.50;
            }
            else {
                title_y -= 0.50;
            }
            if(title_y >= 150) {
                title_lowering = false;
            }
            else if(title_y <= 100) {
                title_lowering = true;
            }
        }
        SDL_RenderPresent(renderer);
        if(SDL_GetTicks() - starting_time < frame_length_ms) {
            SDL_Delay(frame_length_ms - (SDL_GetTicks() - starting_time));
        }
    }
    if(audio_active) {
        destroy_audio(theme_audio);
        destroy_audio(jump_audio);
        SDL_CloseAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);        
    }
    if(l_font != NULL) {
        TTF_DestroyText(titleptr);
        TTF_DestroyText(pauseptr);
        TTF_DestroyText(deathptr);
        TTF_DestroyText(level_selectptr);
    }
}
void check_for_collisions_preemptively(union object* objects, Uint8 object_count) {
    (*objects).player.on_ground = false;
    objects->player.pushing_platform = NULL;
    for(int i = 3; i < object_count; i++) {
        switch((*(objects + i)).type) {
            case PLATFORM:
                if(check_for_vertical_collision_preemptively(&(*objects).player, &(*(objects + i)).platform.hitbox)) {
                    (objects + i)->platform.collided = true;
                }
                check_for_horizontal_collision_preemptively(&(*objects).player, &(*(objects + i)).platform.hitbox);
                if((objects + i)->platform.movement_direction == 1) {
                    (objects + i)->platform.hitbox.x -= 6;
                    if(check_for_vertical_collision_preemptively(&objects->player, &(objects + i)->platform.hitbox)) {
                        (objects + i)->platform.collided = true;
                    }
                    if(objects->player.hitbox.x + objects->player.hitbox.w <= (objects + i)->platform.hitbox.x + ((objects + i)->platform.hitbox.w / 2) && (check_for_current_collision(&objects->player, &(objects + i)->platform.hitbox) || check_for_horizontal_collision_preemptively(&objects->player, &(objects + i)->platform.hitbox))) {
                        objects->player.pushing_platform = &(objects + i)->platform;
                        objects->player.hitbox.x = objects->player.rect.x = (objects + i)->platform.hitbox.x - objects->player.hitbox.w;                        
                    }
                    (objects + i)->platform.hitbox.x += 6;
                }
                else if((objects + i)->platform.movement_direction == 2) {
                    (objects + i)->platform.hitbox.x += 6;
                    if(check_for_vertical_collision_preemptively(&objects->player, &(objects + i)->platform.hitbox)) {
                        (objects + i)->platform.collided = true;
                    }
                    if(objects->player.hitbox.x >= (objects + i)->platform.hitbox.x + ((objects + i)->platform.hitbox.w / 2) && (check_for_current_collision(&objects->player, &(objects + i)->platform.hitbox) || check_for_horizontal_collision_preemptively(&objects->player, &(objects + i)->platform.hitbox))) {
                        objects->player.pushing_platform = &(objects + i)->platform;
                        objects->player.hitbox.x = objects->player.rect.x = (objects + i)->platform.hitbox.x + (objects + i)->platform.hitbox.w;                        
                    }
                    (objects + i)->platform.hitbox.x -= 6;
                }
                break;
            case TRAP:
                check_for_vertical_collision_preemptively(&objects->player, &(objects + i)->trap.stick_hitbox);
                if(check_for_current_collision(&objects->player, &(objects + i)->trap.spike_hitbox) || check_for_vertical_collision_preemptively(&objects->player, &(objects + i)->trap.spike_hitbox) || check_for_horizontal_collision_preemptively(&objects->player, &(objects + i)->trap.spike_hitbox)) {
                    objects->player.alive = false;
                }
                break;
            case BARRIER:
                check_for_horizontal_collision_preemptively(&(*objects).player, &(objects + i)->barrier.wall_hitbox);
                if(check_for_current_collision(&objects->player, &(objects + i)->barrier.top_spike_hitbox) || check_for_current_collision(&objects->player, &(objects + i)->barrier.bottom_spike_hitbox) || check_for_vertical_collision_preemptively(&objects->player, &(objects + i)->barrier.top_spike_hitbox) || check_for_horizontal_collision_preemptively(&objects->player, &(objects + i)->barrier.top_spike_hitbox) || check_for_vertical_collision_preemptively(&objects->player, &(objects + i)->barrier.bottom_spike_hitbox) || check_for_horizontal_collision_preemptively(&objects->player, &(objects + i)->barrier.bottom_spike_hitbox)) {
                    objects->player.alive = false;
                }
                if(check_for_current_collision(&objects->player, &(objects + i)->barrier.wall_hitbox)) {
                    if(objects->player.pushing_platform != NULL) {
                        if(objects->player.pushing_platform->movement_direction == 1) {
                            objects->player.pushing_platform->movement_direction = 2;
                            objects->player.hitbox.x = objects->player.rect.x = (objects + i)->barrier.wall_hitbox.x + (objects + i)->barrier.wall_hitbox.w;
                        }
                        else {
                            objects->player.pushing_platform->movement_direction = 1;
                            objects->player.hitbox.x = objects->player.rect.x = (objects + i)->barrier.wall_hitbox.x - objects->player.hitbox.w;
                        } 
                    }
                    else {
                        objects->player.alive = false;
                    }
                }
                break;
            case ENEMY:
                if(check_for_current_collision(&objects->player, &(objects + i)->enemy.hitbox) || check_for_vertical_collision_preemptively(&objects->player, &(objects + i)->enemy.hitbox) || check_for_horizontal_collision_preemptively(&objects->player, &(objects + i)->enemy.hitbox)) {
                    objects->player.alive = false;
                }
                break;
            case EXIT:
                if(check_for_current_collision(&objects->player, &(objects + i)->exit.hitbox)) {
                    (objects + i)->exit.collided = true;
                }
                break;
        }
    }
    for(int i = 1; i < 3; i++) {
        check_for_horizontal_collision_preemptively(&(*objects).player, &(*(objects + i)).wall.hitbox);
        if(check_for_current_collision(&objects->player, &(objects + i)->wall.hitbox)) {
            if(objects->player.pushing_platform != NULL) {
                if(objects->player.pushing_platform->movement_direction == 1) {
                    objects->player.pushing_platform->movement_direction = 2;
                    objects->player.hitbox.x = objects->player.rect.x = (objects + 1)->wall.hitbox.x + (objects + 1)->wall.hitbox.w;
                }
                else {
                    objects->player.pushing_platform->movement_direction = 1;
                    objects->player.hitbox.x = objects->player.rect.x = (objects + 2)->wall.hitbox.x - objects->player.hitbox.w;
                }
            }
        }        
    }
}
bool check_for_vertical_collision_preemptively(struct player* player, SDL_FRect* hitbox) {
    SDL_FRect player_hitbox = (*player).hitbox;
    float vertical_movement = (*player).vertical_movement;
    Uint8 left_movement = (*player).left_movement;
    Uint8 right_movement = (*player).right_movement;
    SDL_FRect object_hitbox = *hitbox;
    if(player_hitbox.x + player_hitbox.w - left_movement + right_movement > object_hitbox.x && player_hitbox.x - left_movement + right_movement < object_hitbox.x + object_hitbox.w) {
        if(player_hitbox.y + player_hitbox.h <= object_hitbox.y && player_hitbox.y + player_hitbox.h + vertical_movement >= object_hitbox.y) {
            (*player).hitbox.y = (*player).rect.y = object_hitbox.y - player_hitbox.h;
            (*player).on_ground = true;
            (*player).can_jump = true;
            (*player).jump_count = 3;
            (*player).vertical_movement = 0;
            return true;
        }
        else if(player_hitbox.y >= object_hitbox.y + object_hitbox.h && player_hitbox.y + vertical_movement <= object_hitbox.y + object_hitbox.h) {
            (*player).hitbox.y = (*player).rect.y = object_hitbox.y + object_hitbox.h;
            (*player).vertical_movement = 0;
            return true;
        }        
    }
    return false;
}
bool check_for_horizontal_collision_preemptively(struct player* player, SDL_FRect* hitbox) {
    SDL_FRect player_hitbox = (*player).hitbox;
    Uint8 left_movement = (*player).left_movement;
    Uint8 right_movement = (*player).right_movement;
    SDL_FRect object_hitbox = *hitbox;
    if(player_hitbox.y + player_hitbox.h >= object_hitbox.y && player_hitbox.y <= object_hitbox.y + object_hitbox.h) {
        if(player_hitbox.x >= object_hitbox.x + object_hitbox.w && player_hitbox.x - left_movement < object_hitbox.x + object_hitbox.w) {
            (*player).hitbox.x = (*player).rect.x = object_hitbox.x + object_hitbox.w;
            (*player).left_movement = 0;
            return true;
        }
        else if(player_hitbox.x + player_hitbox.w <= object_hitbox.x && player_hitbox.x + player_hitbox.w + right_movement > object_hitbox.x) {
            (*player).hitbox.x = (*player).rect.x = object_hitbox.x - player_hitbox.w;
            (*player).right_movement = 0;
            return true;
        }
    }
    return false;
}
bool check_for_current_collision(struct player* player, SDL_FRect* hitbox) {
    SDL_FRect player_hitbox = player->hitbox;
    SDL_FRect object_hitbox = *hitbox;
    if(player_hitbox.x + player_hitbox.w > object_hitbox.x && player_hitbox.x < object_hitbox.x + object_hitbox.w && player_hitbox.y + player_hitbox.h > object_hitbox.y && player_hitbox.y < object_hitbox.y + object_hitbox.h) {
        if(player_hitbox.x >= object_hitbox.x + (object_hitbox.w / 2)) {
            player->left_movement = 0;
        }
        else {
            player->right_movement = 0;
        }
        return true;   
    }
    return false;
}
void reset_score_label(TTF_Text** score_label, TTF_Font* font, TTF_TextEngine* text_engine, Uint32* score) {
    TTF_DestroyText(*score_label);
    *score_label = (font != NULL) ? TTF_CreateText(text_engine, font, "0", 0) : NULL;
    *score = 0;
}