#include "game_audio.h"
#include <SDL3/SDL.h>
#include <stdio.h>

struct audio* create_audio(const char* pathname, SDL_AudioDeviceID device_id) {
    struct audio* audio = SDL_malloc(sizeof(struct audio));
    SDL_AudioSpec device_spec;
    SDL_LoadWAV(pathname, &(*audio).audio_spec, &(*audio).audio_data, &(*audio).audio_length);
    SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &device_spec, NULL);
    (*audio).audio_stream = SDL_CreateAudioStream(&(*audio).audio_spec, &device_spec);
    SDL_BindAudioStream(device_id, (*audio).audio_stream);
    return audio;
}
void destroy_audio(struct audio* audio) {
    if(audio == NULL) {
        return;
    }
    SDL_free((*audio).audio_data);
    SDL_DestroyAudioStream((*audio).audio_stream);
    SDL_free(audio);
}
void play_audio(struct audio* audio) {
    SDL_PauseAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);
    SDL_ClearAudioStream((*audio).audio_stream);
    SDL_PutAudioStreamData((*audio).audio_stream, (*audio).audio_data, (*audio).audio_length);
    SDL_ResumeAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);
}
void loop_if_done(struct audio* audio) {
    //using 100 here might not work for longer audio files
    if(SDL_GetAudioStreamQueued((*audio).audio_stream) <= 100) {
        play_audio(audio);
    }
}