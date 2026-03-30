#include "game_graphics.h"
#include "game_objects.h"
#include <stdio.h>

void add_constant_object_members(union object* objects, Uint8 object_count);
bool load_objects(union object** objects, const Uint8 level, Uint8* object_count, TTF_TextEngine* text_engine, TTF_Font* font, bool in_mod) {
    if(*objects != NULL) {
        for(int i = 0; i < *object_count; i++) {
            if((*objects + i)->type == HINT && font != NULL) {
                TTF_DestroyText((*objects + i)->hint.text);
            }
        }
        SDL_free(*objects);
        *objects = NULL;
    }
    switch(level) {
        case 0:
            //1 player, 2 walls, 4 platforms, 2 traps, 1 barrier, 2 enemies
            *object_count = 12;
            *objects = SDL_malloc(sizeof(union object) * (*object_count));
            (*objects)->player.type = PLAYER;
            (*objects)->player.hitbox.w = VIRTUAL_SCREEN_WIDTH / 20;
            (*objects)->player.hitbox.h = VIRTUAL_SCREEN_HEIGHT / 10;
            (*objects)->player.hitbox.x = (VIRTUAL_SCREEN_WIDTH / 2) - ((*objects)->player.hitbox.w / 2);
            (*objects)->player.hitbox.y = (VIRTUAL_SCREEN_HEIGHT / 2) + (*objects)->player.hitbox.h;
            for(int i = 1; i < 3; i++) {
                (*(*objects + i)).wall.type = WALL;
                (*(*objects + i)).wall.hitbox.x = (i - 1) * (VIRTUAL_SCREEN_WIDTH - (*objects)->player.hitbox.w);
                (*(*objects + i)).wall.hitbox.w = (*objects)->player.hitbox.w;
            }
            int platform_y = (*objects)->player.hitbox.y + (*objects)->player.hitbox.h;
            for(int i = 3; i < 7; i++) {
                (*(*objects + i)).platform.type = PLATFORM;
                (*(*objects + i)).platform.hitbox.w = (*objects)->player.hitbox.w * 3;
                (*(*objects + i)).platform.hitbox.h = (*objects)->player.hitbox.h / 2;
                if(i == 3) {
                    (*(*objects + i)).platform.hitbox.x = VIRTUAL_SCREEN_WIDTH / 2 - (*(*objects + i)).platform.hitbox.w / 2;
                }
                else {
                    (*(*objects + i)).platform.hitbox.x = SDL_rand(VIRTUAL_SCREEN_WIDTH - (*objects)->player.hitbox.w * 2 - (*(*objects + i)).platform.hitbox.w) + (*objects)->player.hitbox.w;                    
                }
                (*(*objects + i)).platform.hitbox.y = platform_y;
                platform_y -= (*objects)->player.hitbox.h * 3;
                (*(*objects + i)).platform.movement_direction = (*(*objects + i)).platform.shrinking = 0;
            }
            for(int i = 7; i < 9; i++) {
                (*(*objects + i)).trap.type = TRAP;
                (*(*objects + i)).trap.stick_hitbox.x = (i % 2 == 0) ? 0 : 1;
                (*(*objects + i)).trap.stick_hitbox.y = (i % 2 == 0) ? VIRTUAL_SCREEN_HEIGHT * -6 : VIRTUAL_SCREEN_HEIGHT * -8;
                (*(*objects + i)).trap.stick_hitbox.h = (*(*objects + 3)).platform.hitbox.h;
                (*(*objects + i)).trap.cycle = SDL_rand(UINT8_MAX);
            }
            (*(*objects + 9)).barrier.type = BARRIER;
            (*(*objects + 9)).barrier.wall_hitbox.w = (*objects)->player.hitbox.w;
            (*(*objects + 9)).barrier.wall_hitbox.h = (*objects)->player.hitbox.h * 5;
            (*(*objects + 9)).barrier.wall_hitbox.x = SDL_rand(VIRTUAL_SCREEN_WIDTH - (*objects)->player.hitbox.w * 13) + (*objects)->player.hitbox.w * 6;
            (*(*objects + 9)).barrier.wall_hitbox.y = VIRTUAL_SCREEN_HEIGHT * -9;
            for(int i = 10; i < *object_count; i++) {
                (*(*objects + i)).enemy.type = ENEMY;
                (*(*objects + i)).enemy.hitbox.w = (*objects)->player.hitbox.w / 2;
                (*(*objects + i)).enemy.hitbox.h = (*objects)->player.hitbox.h / 2;
                (*(*objects + i)).enemy.hitbox.x = SDL_rand(VIRTUAL_SCREEN_WIDTH - (*objects)->player.hitbox.w * 2 - (*(*objects + i)).enemy.hitbox.w) + (*objects)->player.hitbox.w;
                (*(*objects + i)).enemy.hitbox.y = (i % 2 == 0) ? VIRTUAL_SCREEN_HEIGHT * -13 : VIRTUAL_SCREEN_HEIGHT * -15;
                (*(*objects + i)).enemy.cycle = SDL_rand(UINT8_MAX);
            }
            break;
        default:
            char pathname[18];
            if(in_mod) {
                sprintf(pathname, "./ModLevels/level%u", level);
            }
            else {
                sprintf(pathname, "./Levels/level%u", level);
            }
            SDL_IOStream* level_file_stream = SDL_IOFromFile(pathname, "r");
            if(level_file_stream == NULL) {
                return false;
            }
            SDL_ReadU8(level_file_stream, object_count);
            Uint8 remaining_objects = *object_count;
            Uint8 current_index = 0;
            *objects = SDL_malloc(sizeof(union object) * *object_count);
            Uint16 player_x, player_y, player_w, player_h;
            (*objects)->player.type = PLAYER;
            SDL_ReadU16BE(level_file_stream, &player_x);
            (*objects)->player.hitbox.x = player_x;
            SDL_ReadU16BE(level_file_stream, &player_y);
            (*objects)->player.hitbox.y = player_y;
            SDL_ReadU16BE(level_file_stream, &player_w);
            (*objects)->player.hitbox.w = player_w;
            SDL_ReadU16BE(level_file_stream, &player_h);
            (*objects)->player.hitbox.h = player_h;
            for(int i = 1; i < 3; i++) {
                Uint16 wall_x;
                Uint16 wall_w;
                ((*objects) + i)->wall.type = WALL;
                SDL_ReadU16BE(level_file_stream, &wall_x);
                ((*objects) + i)->wall.hitbox.x = wall_x;
                SDL_ReadU16BE(level_file_stream, &wall_w);
                ((*objects) + i)->wall.hitbox.w = wall_w;
            }
            current_index = 3;
            remaining_objects -= 3;
            if(remaining_objects > 0) {
                Uint8 platform_count;
                Uint8 repetitions;
                Uint16 platform_width;
                Uint16 platform_height;
                Uint8 movement_direction;
                Uint8 shrinking;
                Uint16 platform_x;
                Sint32 platform_y;
                SDL_ReadU8(level_file_stream, &platform_count);
                if(platform_count > 0) {
                    for(int i = current_index; i < current_index + platform_count; i += repetitions) {
                        SDL_ReadU8(level_file_stream, &repetitions);
                        SDL_ReadU16BE(level_file_stream, &platform_width);
                        SDL_ReadU16BE(level_file_stream, &platform_height);
                        SDL_ReadU8(level_file_stream, &movement_direction);
                        SDL_ReadU8(level_file_stream, &shrinking);
                        for(int j = i; j < i + repetitions; j++) {
                            ((*objects) + j)->platform.type = PLATFORM;
                            ((*objects) + j)->platform.hitbox.w = platform_width;
                            ((*objects) + j)->platform.hitbox.h = platform_height;
                            ((*objects) + j)->platform.movement_direction = movement_direction;
                            ((*objects) + j)->platform.shrinking = shrinking;
                            SDL_ReadU16BE(level_file_stream, &platform_x);
                            ((*objects) + j)->platform.hitbox.x = platform_x;
                            SDL_ReadS32BE(level_file_stream, &platform_y);
                            ((*objects) + j)->platform.hitbox.y = platform_y;
                        }
                    }
                    current_index += platform_count;
                    remaining_objects -= platform_count;
                }
            }
            else {
                goto close_file;
            }
            if(remaining_objects > 0) {
                Uint8 trap_count;
                Uint8 repetitions;
                Uint8 side;
                Uint16 trap_stick_height;
                Uint8 cycle;
                Sint32 trap_y;
                SDL_ReadU8(level_file_stream, &trap_count);
                if(trap_count > 0) {
                    for(int i = current_index; i < current_index + trap_count; i += repetitions) {
                        SDL_ReadU8(level_file_stream, &repetitions);
                        SDL_ReadU8(level_file_stream, &side);
                        SDL_ReadU16BE(level_file_stream, &trap_stick_height);
                        SDL_ReadU8(level_file_stream, &cycle);
                        for(int j = i; j < i + repetitions; j++) {
                            ((*objects) + j)->trap.type = TRAP;
                            ((*objects) + j)->trap.stick_hitbox.x = side;
                            ((*objects) + j)->trap.stick_hitbox.h = trap_stick_height;
                            ((*objects) + j)->trap.cycle = cycle;
                            SDL_ReadS32BE(level_file_stream, &trap_y);
                            ((*objects) + j)->trap.stick_hitbox.y = trap_y;
                        }
                    }
                    current_index += trap_count;
                    remaining_objects -= trap_count;
                }
            }
            else {
                goto close_file;
            }
            if(remaining_objects > 0) {
                Uint8 barrier_count;
                Uint8 repetitions;
                Uint16 barrier_x;
                Uint16 barrier_width;
                Uint16 barrier_height;
                Sint32 barrier_y;
                SDL_ReadU8(level_file_stream, &barrier_count);
                if(barrier_count > 0) {
                    for(int i = current_index; i < current_index + barrier_count; i += repetitions) {
                        SDL_ReadU8(level_file_stream, &repetitions);
                        SDL_ReadU16BE(level_file_stream, &barrier_x);
                        SDL_ReadU16BE(level_file_stream, &barrier_width);
                        SDL_ReadU16BE(level_file_stream, &barrier_height);
                        for(int j = i; j < i + repetitions; j++) {
                            ((*objects) + j)->barrier.type = BARRIER;
                            ((*objects) + j)->barrier.wall_hitbox.x = barrier_x;
                            ((*objects) + j)->barrier.wall_hitbox.w = barrier_width;
                            ((*objects) + j)->barrier.wall_hitbox.h = barrier_height;
                            SDL_ReadS32BE(level_file_stream, &barrier_y);
                            ((*objects) + j)->barrier.wall_hitbox.y = barrier_y;
                        }
                    }
                    current_index += barrier_count;
                    remaining_objects -= barrier_count;
                }
            }
            else {
                goto close_file;
            }
            if(remaining_objects > 0) {
                Uint8 enemy_count;
                Uint8 repetitions;
                Uint16 enemy_x;
                Uint16 enemy_width;
                Uint16 enemy_height;
                Uint8 cycle;
                Sint32 enemy_y;
                SDL_ReadU8(level_file_stream, &enemy_count);
                if(enemy_count > 0) {
                    for(int i = current_index; i < current_index + enemy_count; i += repetitions) {
                        SDL_ReadU8(level_file_stream, &repetitions);
                        SDL_ReadU16BE(level_file_stream, &enemy_width);
                        SDL_ReadU16BE(level_file_stream, &enemy_height);
                        SDL_ReadU8(level_file_stream, &cycle);
                        for(int j = i; j < i + repetitions; j++) {
                            ((*objects) + j)->enemy.type = ENEMY;
                            ((*objects) + j)->enemy.hitbox.w = enemy_width;
                            ((*objects) + j)->enemy.hitbox.h = enemy_height;
                            ((*objects) + j)->enemy.cycle = cycle;
                            SDL_ReadU16BE(level_file_stream, &enemy_x);
                            SDL_ReadS32BE(level_file_stream, &enemy_y);
                            ((*objects) + j)->enemy.hitbox.x = enemy_x;
                            ((*objects) + j)->enemy.hitbox.y = enemy_y;
                        }
                    }
                    current_index += enemy_count;
                    remaining_objects -= enemy_count;
                }
            }
            else {
                goto close_file;
            }
            if(remaining_objects > 0) {
                Uint8 exit_count;
                Uint8 repetitions;
                Uint16 exit_x;
                Uint16 exit_width;
                Uint16 exit_height;
                Sint32 exit_y;
                SDL_ReadU8(level_file_stream, &exit_count);
                if(exit_count > 0) {
                    for(int i = current_index; i < current_index + exit_count; i += repetitions) {
                        SDL_ReadU8(level_file_stream, &repetitions);
                        SDL_ReadU16BE(level_file_stream, &exit_x);
                        SDL_ReadU16BE(level_file_stream, &exit_width);
                        SDL_ReadU16BE(level_file_stream, &exit_height);
                        for(int j = i; j < i + repetitions; j++) {
                            ((*objects) + j)->exit.type = EXIT;
                            ((*objects) + j)->exit.hitbox.x = exit_x;
                            ((*objects) + j)->exit.hitbox.w = exit_width;
                            ((*objects) + j)->exit.hitbox.h = exit_height;
                            SDL_ReadS32BE(level_file_stream, &exit_y);
                            ((*objects) + j)->exit.hitbox.y = exit_y;
                            SDL_ReadU8(level_file_stream, &((*objects) + j)->exit.exit_level);
                        }
                    }
                    current_index += exit_count;
                    remaining_objects -= exit_count;
                }
            }
            else {
                goto close_file;
            }
            if(remaining_objects > 0) {
                Uint8 hint_count;
                Uint8 length;
                char hint[256];
                Uint16 x;
                Sint32 y;
                SDL_ReadU8(level_file_stream, &hint_count);
                if(hint_count > 0) {
                    for(int i = current_index; i < current_index + hint_count; i++) {
                        SDL_ReadU8(level_file_stream, &length);
                        SDL_ReadIO(level_file_stream, &hint, length);
                        hint[length] = '\0';
                        SDL_ReadU16BE(level_file_stream, &x);
                        SDL_ReadS32BE(level_file_stream, &y);
                        ((*objects) + i)->hint.type = HINT;
                        ((*objects) + i)->hint.text = TTF_CreateText(text_engine, font, hint, 0);
                        ((*objects) + i)->hint.x = x;
                        ((*objects) + i)->hint.y = y;
                    }
                    current_index += hint_count;
                    remaining_objects -= hint_count;
                }
            }
            close_file:
            SDL_CloseIO(level_file_stream);
            break;
    }
    add_constant_object_members(*objects, *object_count);
    return true;
}
void add_constant_object_members(union object* objects, Uint8 object_count) {
    objects->player.rect.x = objects->player.hitbox.x;
    objects->player.rect.y = objects->player.hitbox.y;
    objects->player.rect.w = objects->player.hitbox.w;
    objects->player.rect.h = objects->player.hitbox.h;
    objects->player.vertical_movement = objects->player.left_movement = objects->player.right_movement = 0;
    objects->player.on_ground = objects->player.can_jump = false;
    objects->player.alive = true;
    objects->player.jump_count = 3;
    objects->player.pushing_platform = NULL;
    for(int i = 1; i < 3; i++) {
        (objects + i)->wall.rect.x = (objects + i)->wall.hitbox.x;
        (objects + i)->wall.rect.y = (objects + i)->wall.hitbox.y = 0;
        (objects + i)->wall.rect.w = (objects + i)->wall.hitbox.w;
        (objects + i)->wall.rect.h = (objects + i)->wall.hitbox.h = VIRTUAL_SCREEN_HEIGHT;
    }
    for(int i = 3; i < object_count; i++) {
        switch((objects + i)->type) {
            case PLATFORM:
                (objects + i)->platform.rect.x = (objects + i)->platform.hitbox.x;
                (objects + i)->platform.rect.y = (objects + i)->platform.hitbox.y;
                (objects + i)->platform.rect.w = (objects + i)->platform.hitbox.w;
                (objects + i)->platform.rect.h = (objects + i)->platform.hitbox.h;
                (objects + i)->platform.collided = false;
                break;
            case TRAP:
                (objects + i)->trap.stick_rect.x = (objects + i)->trap.stick_hitbox.x;
                (objects + i)->trap.stick_rect.y = (objects + i)->trap.stick_hitbox.y;
                (objects + i)->trap.stick_rect.h = (objects + i)->trap.stick_hitbox.h;
                (objects + i)->trap.stick_hitbox.w = (objects + i)->trap.stick_rect.w = ((objects + i)->trap.cycle < 128) ? (objects + i)->trap.cycle * 4 : (128 - (objects + i)->trap.cycle % 128) * 4;
                (objects + i)->trap.stick_hitbox.x = (objects + i)->trap.stick_rect.x = ((objects + i)->trap.stick_hitbox.x == 0) ? (objects + 1)->wall.hitbox.x + (objects + 1)->wall.hitbox.w : (objects + 2)->wall.hitbox.x - (objects + i)->trap.stick_hitbox.w;
                (objects + i)->trap.spike_hitbox.y = (objects + i)->trap.stick_hitbox.y;
                (objects + i)->trap.spike_hitbox.w = (objects + i)->trap.stick_hitbox.h;
                (objects + i)->trap.spike_hitbox.h = (objects + i)->trap.stick_hitbox.h;
                if((objects + i)->trap.stick_hitbox.x == (objects + 1)->wall.hitbox.x + (objects + 1)->wall.hitbox.w) {
                    (objects + i)->trap.spike_hitbox.x = (objects + i)->trap.stick_hitbox.x + (objects + i)->trap.stick_hitbox.w;
                    (objects + i)->trap.spike_vertices[0].position.x = (objects + i)->trap.spike_vertices[1].position.x = (objects + i)->trap.spike_hitbox.x;
                    (objects + i)->trap.spike_vertices[2].position.x = (objects + i)->trap.spike_hitbox.x + (objects + i)->trap.spike_hitbox.w;
                }
                else {
                    (objects + i)->trap.spike_hitbox.x = (objects + i)->trap.stick_hitbox.x - (objects + i)->trap.spike_hitbox.w;
                    (objects + i)->trap.spike_vertices[0].position.x = (objects + i)->trap.spike_vertices[1].position.x = (objects + i)->trap.spike_hitbox.x + (objects + i)->trap.spike_hitbox.w;
                    (objects + i)->trap.spike_vertices[2].position.x = (objects + i)->trap.spike_hitbox.x;
                }
                (objects + i)->trap.spike_vertices[0].position.y = (objects + i)->trap.spike_hitbox.y;
                (objects + i)->trap.spike_vertices[1].position.y = (objects + i)->trap.spike_hitbox.y + (objects + i)->trap.spike_hitbox.h;
                (objects + i)->trap.spike_vertices[2].position.y = (objects + i)->trap.spike_hitbox.y + (objects + i)->trap.spike_hitbox.h / 2;
                for(int j = 0; j < 3; j++) {
                    (objects + i)->trap.spike_vertices[j].color.r = 0.8;
                    (objects + i)->trap.spike_vertices[j].color.g = 0.8;
                    (objects + i)->trap.spike_vertices[j].color.b = 0.8;
                    (objects + i)->trap.spike_vertices[j].color.a = 1.0;
                }
                break;
            case BARRIER:
                (objects + i)->barrier.wall_rect.x = (objects + i)->barrier.wall_hitbox.x;
                (objects + i)->barrier.wall_rect.y = (objects + i)->barrier.wall_hitbox.y;
                (objects + i)->barrier.wall_rect.w = (objects + i)->barrier.wall_hitbox.w;
                (objects + i)->barrier.wall_rect.h = (objects + i)->barrier.wall_hitbox.h;
                (objects + i)->barrier.top_spike_hitbox.x = (objects + i)->barrier.bottom_spike_hitbox.x = (objects + i)->barrier.wall_hitbox.x;
                (objects + i)->barrier.top_spike_hitbox.w = (objects + i)->barrier.bottom_spike_hitbox.w = (objects + i)->barrier.wall_hitbox.w;
                (objects + i)->barrier.top_spike_hitbox.h = (objects + i)->barrier.bottom_spike_hitbox.h = (objects + i)->barrier.top_spike_hitbox.w;
                (objects + i)->barrier.top_spike_hitbox.y = (objects + i)->barrier.wall_hitbox.y - (objects + i)->barrier.top_spike_hitbox.w;
                (objects + i)->barrier.bottom_spike_hitbox.y = (objects + i)->barrier.wall_hitbox.y + (objects + i)->barrier.wall_hitbox.h;
                (objects + i)->barrier.top_spike_vertices[0].position.x = (objects + i)->barrier.bottom_spike_vertices[0].position.x = (objects + i)->barrier.wall_hitbox.x;
                (objects + i)->barrier.top_spike_vertices[1].position.x = (objects + i)->barrier.bottom_spike_vertices[1].position.x = (objects + i)->barrier.wall_hitbox.x + (objects + i)->barrier.wall_hitbox.w;
                (objects + i)->barrier.top_spike_vertices[2].position.x = (objects + i)->barrier.bottom_spike_vertices[2].position.x = (objects + i)->barrier.wall_hitbox.x + (objects + i)->barrier.wall_hitbox.w / 2;
                (objects + i)->barrier.top_spike_vertices[0].position.y = (objects + i)->barrier.wall_hitbox.y;
                (objects + i)->barrier.top_spike_vertices[1].position.y = (objects + i)->barrier.wall_hitbox.y;
                (objects + i)->barrier.top_spike_vertices[2].position.y = (objects + i)->barrier.top_spike_hitbox.y;
                (objects + i)->barrier.bottom_spike_vertices[0].position.y = (objects + i)->barrier.bottom_spike_hitbox.y;
                (objects + i)->barrier.bottom_spike_vertices[1].position.y = (objects + i)->barrier.bottom_spike_hitbox.y;
                (objects + i)->barrier.bottom_spike_vertices[2].position.y = (objects + i)->barrier.bottom_spike_hitbox.y + (objects + i)->barrier.bottom_spike_hitbox.h;
                (objects + i)->barrier.top_spike_hitbox.x = (objects + i)->barrier.bottom_spike_hitbox.x += (objects + i)->barrier.top_spike_hitbox.w / 16;
                (objects + i)->barrier.top_spike_hitbox.y += (objects + i)->barrier.top_spike_hitbox.h / 16;
                (objects + i)->barrier.top_spike_hitbox.w = (objects + i)->barrier.bottom_spike_hitbox.w *= 0.875;
                (objects + i)->barrier.top_spike_hitbox.h = (objects + i)->barrier.bottom_spike_hitbox.h *= 0.9375;
                for(int j = 0; j < 3; j++) {
                    (objects + i)->barrier.top_spike_vertices[j].color.r = 0.8;
                    (objects + i)->barrier.top_spike_vertices[j].color.g = 0.8;
                    (objects + i)->barrier.top_spike_vertices[j].color.b = 0.8;
                    (objects + i)->barrier.top_spike_vertices[j].color.a = 1.0;
                    (objects + i)->barrier.bottom_spike_vertices[j].color.r = 0.8;
                    (objects + i)->barrier.bottom_spike_vertices[j].color.g = 0.8;
                    (objects + i)->barrier.bottom_spike_vertices[j].color.b = 0.8;
                    (objects + i)->barrier.bottom_spike_vertices[j].color.a = 1.0;
                }
                break;
            case ENEMY:
                (objects + i)->enemy.vertices[0].position.x = (objects + i)->enemy.hitbox.x;
                (objects + i)->enemy.vertices[1].position.x = (objects + i)->enemy.vertices[2].position.x = (objects + i)->enemy.vertices[6].position.x = (objects + i)->enemy.vertices[7].position.x = (objects + i)->enemy.vertices[11].position.x = (objects + i)->enemy.hitbox.x + (objects + i)->enemy.hitbox.w / 3;
                (objects + i)->enemy.vertices[3].position.x = (objects + i)->enemy.hitbox.x + (objects + i)->enemy.hitbox.w;
                (objects + i)->enemy.vertices[4].position.x = (objects + i)->enemy.vertices[5].position.x = (objects + i)->enemy.vertices[8].position.x = (objects + i)->enemy.vertices[9].position.x = (objects + i)->enemy.vertices[10].position.x = (objects + i)->enemy.hitbox.x + ((objects + i)->enemy.hitbox.w / 3) * 2;
                (objects + i)->enemy.vertices[0].position.y = (objects + i)->enemy.vertices[3].position.y = (objects + i)->enemy.hitbox.y + (objects + i)->enemy.hitbox.h / 2;
                (objects + i)->enemy.vertices[1].position.y = (objects + i)->enemy.vertices[4].position.y = (objects + i)->enemy.vertices[6].position.y = (objects + i)->enemy.vertices[8].position.y = (objects + i)->enemy.vertices[10].position.y = (objects + i)->enemy.hitbox.y;
                (objects + i)->enemy.vertices[2].position.y = (objects + i)->enemy.vertices[5].position.y = (objects + i)->enemy.vertices[7].position.y = (objects + i)->enemy.vertices[9].position.y = (objects + i)->enemy.vertices[11].position.y = (objects + i)->enemy.hitbox.y + (objects + i)->enemy.hitbox.h;
                (objects + i)->enemy.hitbox.x += (objects + i)->enemy.hitbox.w / 8;
                (objects + i)->enemy.hitbox.y += (objects + i)->enemy.hitbox.h / 8;
                (objects + i)->enemy.hitbox.w *= 0.75;
                (objects + i)->enemy.hitbox.h *= 0.75;
                for(int j = 0; j < 12; j++) {
                    (objects + i)->enemy.vertices[j].color.r = 1.0;
                    (objects + i)->enemy.vertices[j].color.g = 0.4;
                    (objects + i)->enemy.vertices[j].color.b = 0.2;
                    (objects + i)->enemy.vertices[j].color.a = 1.0;
                }
                break;
            case EXIT:
                (objects + i)->exit.vertices[0].position.x = (objects + i)->exit.vertices[1].position.x = (objects + i)->exit.vertices[5].position.x = (objects + i)->exit.vertices[6].position.x = (objects + i)->exit.vertices[9].position.x = (objects + i)->exit.hitbox.x;
                (objects + i)->exit.vertices[2].position.x = (objects + i)->exit.vertices[3].position.x = (objects + i)->exit.vertices[4].position.x = (objects + i)->exit.vertices[10].position.x = (objects + i)->exit.hitbox.x + (objects + i)->exit.hitbox.w;
                (objects + i)->exit.vertices[8].position.x = (objects + i)->exit.hitbox.x + (objects + i)->exit.hitbox.w / 4;
                (objects + i)->exit.vertices[7].position.x = (objects + i)->exit.vertices[11].position.x = (objects + i)->exit.hitbox.x + ((objects + i)->exit.hitbox.w / 4) * 3;
                (objects + i)->exit.vertices[8].position.y = (objects + i)->exit.vertices[7].position.y = (objects + i)->exit.vertices[11].position.y = (objects + i)->exit.hitbox.y;
                (objects + i)->exit.vertices[0].position.y = (objects + i)->exit.vertices[2].position.y = (objects + i)->exit.vertices[3].position.y = (objects + i)->exit.hitbox.y + (objects + i)->exit.hitbox.h;
                (objects + i)->exit.vertices[1].position.y = (objects + i)->exit.vertices[4].position.y = (objects + i)->exit.vertices[5].position.y = (objects + i)->exit.vertices[6].position.y = (objects + i)->exit.vertices[10].position.y = (objects + i)->exit.vertices[9].position.y = (objects + i)->exit.hitbox.y + (objects + i)->exit.hitbox.h / 4;
                for(int j = 0; j < 12; j++) {
                    (objects + i)->exit.vertices[j].color.r = 0.0;
                    (objects + i)->exit.vertices[j].color.g = 0.0;
                    (objects + i)->exit.vertices[j].color.b = 0.0;
                    (objects + i)->exit.vertices[j].color.a = 1.0;
                }
                (objects + i)->exit.collided = false;
                (objects + i)->exit.hitbox.x = (objects + i)->exit.hitbox.x + ((objects + i)->exit.hitbox.w / 2);
                (objects + i)->exit.hitbox.w = 1;
                break;
        }
    }
}
void move_objects(union object* objects, Uint8 object_count, int displacement) {
    if(displacement != 0) {
        for(int i = 3; i < object_count; i++) {
            switch((objects + i)->type) {
                case PLATFORM:
                    (objects + i)->platform.hitbox.y = (objects + i)->platform.rect.y += displacement;
                    break;
                case TRAP:
                    (objects + i)->trap.stick_hitbox.y = (objects + i)->trap.stick_rect.y += displacement;
                    (objects + i)->trap.spike_hitbox.y += displacement;
                    for(int j = 0; j < 3; j++) {
                        (objects + i)->trap.spike_vertices[j].position.y += displacement;
                    }
                    break;
                case BARRIER:
                    (objects + i)->barrier.wall_hitbox.y = (objects + i)->barrier.wall_rect.y += displacement;
                    (objects + i)->barrier.top_spike_hitbox.y += displacement;
                    (objects + i)->barrier.bottom_spike_hitbox.y += displacement;
                    for(int j = 0; j < 3; j++) {
                        (objects + i)->barrier.top_spike_vertices[j].position.y += displacement;
                        (objects + i)->barrier.bottom_spike_vertices[j].position.y += displacement;
                    }
                    break;
                case ENEMY:
                    (objects + i)->enemy.hitbox.y += displacement;
                    for(int j = 0; j < 12; j++) {
                        (objects + i)->enemy.vertices[j].position.y += displacement;
                    }
                    break;
                case EXIT:
                    (objects + i)->exit.hitbox.y += displacement;
                    for(int j = 0; j < 12; j++) {
                        (objects + i)->exit.vertices[j].position.y += displacement;
                    }
                    break;
                case HINT:
                    (objects + i)->hint.y += displacement;
                    break;
            }
        }
    }
    for(int i = 3; i < object_count; i++) {
        switch((objects + i)->type) {
            case PLATFORM:
                if((objects + i)->platform.hitbox.w > 0 && (objects + i)->platform.movement_direction == 1) {
                    (objects + i)->platform.hitbox.x = (objects + i)->platform.rect.x -= 6;
                    if((objects + i)->platform.hitbox.x <= (objects + 1)->wall.hitbox.x + (objects + 1)->wall.hitbox.w) {
                        (objects + i)->platform.movement_direction = 2;
                    }
                }
                else if((objects + i)->platform.hitbox.w > 0 && (objects + i)->platform.movement_direction == 2) {
                    (objects + i)->platform.hitbox.x = (objects + i)->platform.rect.x += 6;
                    if((objects + i)->platform.hitbox.x + (objects + i)->platform.hitbox.w >= (objects + 2)->wall.hitbox.x) {
                        (objects + i)->platform.movement_direction = 1;
                    }
                }
                if((objects + i)->platform.collided && (objects + i)->platform.shrinking && (objects + i)->platform.hitbox.w > 0) {
                    (objects + i)->platform.hitbox.w = (objects + i)->platform.rect.w -= 8;
                    (objects + i)->platform.hitbox.x = (objects + i)->platform.rect.x += 4;
                    if((objects + i)->platform.movement_direction == 1) {
                        (objects + i)->platform.hitbox.x = (objects + i)->platform.rect.x += 6;
                    }
                    else if((objects + i)->platform.movement_direction == 2) {
                        (objects + i)->platform.hitbox.x = (objects + i)->platform.rect.x -= 6;
                    }
                    if((objects + i)->platform.hitbox.w <= 0) {
                        (objects + i)->platform.hitbox.x = (objects + i)->platform.rect.x = -1;
                        (objects + i)->platform.hitbox.w = (objects + i)->platform.rect.w = 0;
                    }
                }
                break;
            case TRAP:
                if((objects + i)->trap.stick_hitbox.x == (objects + 1)->wall.hitbox.x + (objects + 1)->wall.hitbox.w) {
                    if((objects + i)->trap.cycle >= 128) {
                        (objects + i)->trap.stick_hitbox.w = (objects + i)->trap.stick_rect.w -= 4;
                        (objects + i)->trap.spike_hitbox.x -= 4;
                        for(int j = 0; j < 3; j++) {
                            (objects + i)->trap.spike_vertices[j].position.x -= 4;
                        }
                    }
                    else {
                        (objects + i)->trap.stick_hitbox.w = (objects + i)->trap.stick_rect.w += 4;
                        (objects + i)->trap.spike_hitbox.x += 4;
                        for(int j = 0; j < 3; j++) {
                            (objects + i)->trap.spike_vertices[j].position.x += 4;
                        }
                    }
                }
                else {
                    if((objects + i)->trap.cycle >= 128) {
                        (objects + i)->trap.stick_hitbox.w = (objects + i)->trap.stick_rect.w -= 4;
                        (objects + i)->trap.stick_hitbox.x = (objects + i)->trap.stick_rect.x += 4;
                        (objects + i)->trap.spike_hitbox.x += 4;
                        for(int j = 0; j < 3; j++) {
                            (objects + i)->trap.spike_vertices[j].position.x += 4;
                        }
                    }
                    else {
                        (objects + i)->trap.stick_hitbox.w = (objects + i)->trap.stick_rect.w += 4;
                        (objects + i)->trap.stick_hitbox.x = (objects + i)->trap.stick_rect.x -= 4;
                        (objects + i)->trap.spike_hitbox.x -= 4;
                        for(int j = 0; j < 3; j++) {
                            (objects + i)->trap.spike_vertices[j].position.x -= 4;
                        }
                    }
                }
                (objects + i)->trap.cycle++;
                break;
            case ENEMY:
                if((objects + i)->enemy.cycle < 128) {
                    (objects + i)->enemy.hitbox.y -= 8;
                    for(int j = 0; j < 12; j++) {
                        (objects + i)->enemy.vertices[j].position.y -= 8;
                    }
                }
                else {
                    (objects + i)->enemy.hitbox.y += 8;
                    for(int j = 0; j < 12; j++) {
                        (objects + i)->enemy.vertices[j].position.y += 8;
                    }
                }
                (objects + i)->enemy.cycle++;
                break;
        }
    }
}