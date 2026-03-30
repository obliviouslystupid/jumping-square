#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#define PLAYER 0
#define WALL 1
#define PLATFORM 2
#define TRAP 3
#define BARRIER 4
#define ENEMY 5
#define EXIT 6
#define HINT 7

struct wall {
    Uint8 type;
    SDL_FRect hitbox;
    SDL_FRect rect;
};

struct platform {
    Uint8 type;
    SDL_FRect hitbox;
    SDL_FRect rect;
    Uint8 movement_direction;
    bool shrinking;
    bool collided;
};

struct player {
    Uint8 type;
    SDL_FRect hitbox;
    SDL_FRect rect;
    Uint8 left_movement;
    Uint8 right_movement;
    float vertical_movement;
    bool on_ground;
    bool can_jump;
    bool alive;
    Uint8 jump_count;
    struct platform* pushing_platform;
};

struct trap {
    Uint8 type;
    SDL_FRect stick_hitbox;
    SDL_FRect spike_hitbox;
    SDL_FRect stick_rect;
    SDL_Vertex spike_vertices[3];
    Uint8 cycle;
};

struct barrier {
    Uint8 type;
    SDL_FRect wall_hitbox;
    SDL_FRect top_spike_hitbox;
    SDL_FRect bottom_spike_hitbox;    
    SDL_FRect wall_rect;
    SDL_Vertex top_spike_vertices[3];
    SDL_Vertex bottom_spike_vertices[3];
};

struct enemy {
    Uint8 type;
    SDL_FRect hitbox;
    SDL_Vertex vertices[12];
    Uint8 cycle;
};

struct exit {
    Uint8 type;
    SDL_FRect hitbox;
    SDL_Vertex vertices[12];
    Uint8 exit_level;
    bool collided;
};

struct hint {
    Uint8 type;
    TTF_Text* text;
    Uint16 x;
    Sint32 y;
};

union object {
    Uint8 type;
    struct player player;
    struct wall wall;
    struct platform platform;
    struct trap trap;
    struct barrier barrier;
    struct enemy enemy;
    struct exit exit;
    struct hint hint;
};

bool load_objects(union object** objects, const Uint8 level, Uint8* object_count, TTF_TextEngine* text_engine, TTF_Font* font, bool in_mod);
void move_objects(union object* objects, Uint8 object_count, int displacement);

#endif