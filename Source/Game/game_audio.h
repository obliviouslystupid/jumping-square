#ifndef GAME_AUDIO_H
#define GAME_AUDIO_H
#include <SDL3/SDL.h>

struct audio {
    SDL_AudioSpec audio_spec;
    Uint8* audio_data;
    Uint32 audio_length;
    SDL_AudioStream* audio_stream;
};

struct audio* create_audio(const char* pathname, SDL_AudioDeviceID device_id);
void destroy_audio(struct audio* audio);
void play_audio(struct audio* audio);
void loop_if_done(struct audio* audio);

#endif