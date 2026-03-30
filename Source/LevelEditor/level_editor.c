#include <SDL3/SDL.h>
#include <stdio.h>

void add_object_count(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* object_count);
void add_player(SDL_IOStream* input_stream, SDL_IOStream* output_stream);
void add_walls(SDL_IOStream* input_stream, SDL_IOStream* output_stream);
bool add_platforms(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* remaining_objects);
bool add_traps(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* remaining_objects);
bool add_barriers(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* remaining_objects);
bool add_enemies(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* remaining_objects);
bool add_exits(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* remaining_objects);
bool add_hints(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* remaining_objects);
Uint8 read_uint8(SDL_IOStream* input_stream);
Uint16 read_uint16(SDL_IOStream* input_stream);
Sint32 read_sint32(SDL_IOStream* input_stream);
void close_io(SDL_IOStream* stream1, SDL_IOStream* stream2);
int main(int argc, char** argv) {
    if(argc != 3) {
        puts("Improper argument count");
        return -1;
    }
    SDL_IOStream* text_file_stream = SDL_IOFromFile(argv[1], "r");
    if(text_file_stream == NULL) {
        puts("Failure reading file");
        return -1;
    }
    SDL_IOStream* output_file_stream = SDL_IOFromFile(argv[2], "w");
    if(output_file_stream == NULL) {
        puts("Failure writing file");
        SDL_CloseIO(text_file_stream);
        return -1;
    }
    int string_index = 0;
    Uint8 object_count;
    Uint8 remaining_objects;
    add_object_count(text_file_stream, output_file_stream, &object_count);
    if(object_count < 4) {
        low_object_count:
        puts("OBJECT COUNT TOO LOW");
        close_io(text_file_stream, output_file_stream);
        return -1;
    }
    remaining_objects = object_count;
    add_player(text_file_stream, output_file_stream);
    add_walls(text_file_stream, output_file_stream);
    remaining_objects -= 3;
    if(!add_platforms(text_file_stream, output_file_stream, &remaining_objects)) {
        goto low_object_count;
    }
    if(!add_traps(text_file_stream, output_file_stream, &remaining_objects)) {
        goto low_object_count;
    }
    if(!add_barriers(text_file_stream, output_file_stream, &remaining_objects)) {
        goto low_object_count;
    }
    if(!add_enemies(text_file_stream, output_file_stream, &remaining_objects)) {
        goto low_object_count;
    }
    if(!add_exits(text_file_stream, output_file_stream, &remaining_objects)) {
        goto low_object_count;
    }
    if(!add_hints(text_file_stream, output_file_stream, &remaining_objects)) {
        goto low_object_count;
    }
    if(remaining_objects != 0) {
        puts("OBJECT COUNT TOO HIGH.\nREWRITING OBJECT COUNT");
        SDL_SeekIO(output_file_stream, 0, SDL_IO_SEEK_SET);
        SDL_WriteU8(output_file_stream, object_count - remaining_objects);
    }
    puts("LEVEL FILE CREATED SUCCESSFULLY!");
    close_io(text_file_stream, output_file_stream);
    return 0;
}
void add_object_count(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* object_count) {
    *object_count = read_uint8(input_stream);
    SDL_WriteU8(output_stream, *object_count);
}
void add_player(SDL_IOStream* input_stream, SDL_IOStream* output_stream) {
    Uint16 value;
    for(int i = 0; i < 4; i++) {
        value = read_uint16(input_stream);
        SDL_WriteU16BE(output_stream, value);
    }
}
void add_walls(SDL_IOStream* input_stream, SDL_IOStream* output_stream) {
    Uint16 x;
    Uint16 width;
    for(int i = 0; i < 2; i++) {
        x = read_uint16(input_stream);
        width = read_uint16(input_stream);
        SDL_WriteU16BE(output_stream, x);
        SDL_WriteU16BE(output_stream, width);
    }
}
bool add_platforms(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* remaining_objects) {
    Uint8 platform_count = read_uint8(input_stream);
    if(platform_count > *remaining_objects) {
        return false;
    }
    SDL_WriteU8(output_stream, platform_count);
    if(platform_count == 0) {
        return true;
    }
    Uint8 repetition;
    Uint16 width;
    Uint16 height;
    Uint8 direction;
    Uint8 shrink;
    Uint16 x;
    Sint32 y;
    for(int i = 0; i < platform_count; i += repetition) {
        repetition = read_uint8(input_stream);
        width = read_uint16(input_stream);
        height = read_uint16(input_stream);
        direction = read_uint8(input_stream);
        shrink = read_uint8(input_stream);
        SDL_WriteU8(output_stream, repetition);
        SDL_WriteU16BE(output_stream, width);
        SDL_WriteU16BE(output_stream, height);
        SDL_WriteU8(output_stream, direction);
        SDL_WriteU8(output_stream, shrink);
        for(int j = 0; j < repetition; j++) {
            x = read_uint16(input_stream);
            y = read_sint32(input_stream);
            SDL_WriteU16BE(output_stream, x);
            SDL_WriteS32BE(output_stream, y);
        }
    }
    *remaining_objects -= platform_count;
    return true;
}
bool add_traps(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* remaining_objects) {
    Uint8 trap_count = read_uint8(input_stream);
    if(trap_count > *remaining_objects) {
        return false;
    }
    SDL_WriteU8(output_stream, trap_count);
    if(trap_count == 0) {
        return true;
    }
    Uint8 repetition;
    Uint8 side;
    Uint16 height;
    Uint8 cycle;
    Sint32 y;
    for(int i = 0; i < trap_count; i += repetition) {
        repetition = read_uint8(input_stream);
        side = read_uint8(input_stream);
        height = read_uint16(input_stream);
        cycle = read_uint8(input_stream);
        SDL_WriteU8(output_stream, repetition);
        SDL_WriteU8(output_stream, side);
        SDL_WriteU16BE(output_stream, height);
        SDL_WriteU8(output_stream, cycle);
        for(int j = 0; j < repetition; j++) {
            y = read_sint32(input_stream);
            SDL_WriteS32BE(output_stream, y);
        }
    }
    *remaining_objects -= trap_count;
    return true;
}
bool add_barriers(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* remaining_objects) {
    Uint8 barrier_count = read_uint8(input_stream);
    if(barrier_count > *remaining_objects) {
        return false;
    }
    SDL_WriteU8(output_stream, barrier_count);
    if(barrier_count == 0) {
        return true;
    }
    Uint8 repetition;
    Uint16 x;
    Uint16 width;
    Uint16 height;
    Sint32 y;
    for(int i = 0; i < barrier_count; i += repetition) {
        repetition = read_uint8(input_stream);
        x = read_uint16(input_stream);
        width = read_uint16(input_stream);
        height = read_uint16(input_stream);
        SDL_WriteU8(output_stream, repetition);
        SDL_WriteU16BE(output_stream, x);
        SDL_WriteU16BE(output_stream, width);
        SDL_WriteU16BE(output_stream, height);
        for(int j = 0; j < repetition; j++) {
            y = read_sint32(input_stream);
            SDL_WriteS32BE(output_stream, y);
        }
    }
    *remaining_objects -= barrier_count;
    return true;
}
bool add_enemies(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* remaining_objects) {
    Uint8 enemy_count = read_uint8(input_stream);
    if(enemy_count > *remaining_objects) {
        return false;
    }
    SDL_WriteU8(output_stream, enemy_count);
    if(enemy_count == 0) {
        return true;
    }
    Uint8 repetition;
    Uint16 width;
    Uint16 height;
    Uint8 cycle;
    Uint16 x;
    Sint32 y;
    for(int i = 0; i < enemy_count; i += repetition) {
        repetition = read_uint8(input_stream);
        width = read_uint16(input_stream);
        height = read_uint16(input_stream);
        cycle = read_uint8(input_stream);
        SDL_WriteU8(output_stream, repetition);
        SDL_WriteU16BE(output_stream, width);
        SDL_WriteU16BE(output_stream, height);
        SDL_WriteU8(output_stream, cycle);
        for(int j = 0; j < repetition; j++) {
            x = read_uint16(input_stream);
            y = read_sint32(input_stream);
            SDL_WriteU16BE(output_stream, x);
            SDL_WriteS32BE(output_stream, y);
        }
    }
    *remaining_objects -= enemy_count;
    return true;
}
bool add_exits(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* remaining_objects) {
    Uint8 exit_count = read_uint8(input_stream);
    if(exit_count > *remaining_objects) {
        return false;
    }
    SDL_WriteU8(output_stream, exit_count);
    if(exit_count == 0) {
        return true;
    }
    Uint8 repetition;
    Uint16 x;
    Uint16 width;
    Uint16 height;
    Sint32 y;
    Uint8 exit_level;
    for(int i = 0; i < exit_count; i += repetition) {
        repetition = read_uint8(input_stream);
        x = read_uint16(input_stream);
        width = read_uint16(input_stream);
        height = read_uint16(input_stream);
        SDL_WriteU8(output_stream, repetition);
        SDL_WriteU16BE(output_stream, x);
        SDL_WriteU16BE(output_stream, width);
        SDL_WriteU16BE(output_stream, height);
        for(int j = 0; j < repetition; j++) {
            y = read_sint32(input_stream);
            SDL_WriteS32BE(output_stream, y);
            exit_level = read_uint8(input_stream);
            SDL_WriteU8(output_stream, exit_level);
        }
    }
    *remaining_objects -= exit_count;
    return true;
}
bool add_hints(SDL_IOStream* input_stream, SDL_IOStream* output_stream, Uint8* remaining_objects) {
    Uint8 hint_count = read_uint8(input_stream);
    if(hint_count > *remaining_objects) {
        return false;
    }
    SDL_WriteU8(output_stream, hint_count);
    if(hint_count == 0) {
        return true;
    }
    Uint8 length;
    char buffer[256];
    Uint16 x;
    Sint32 y;
    for(int i = 0; i < hint_count; i++) {
        length = read_uint8(input_stream);
        SDL_ReadIO(input_stream, buffer, length);
        buffer[length] = '\0';
        x = read_uint16(input_stream);
        y = read_sint32(input_stream);
        SDL_WriteU8(output_stream, length);
        SDL_WriteIO(output_stream, buffer, length);
        SDL_WriteU16BE(output_stream, x);
        SDL_WriteS32BE(output_stream, y);
    }
    *remaining_objects -= hint_count;
    return true;
}
Uint8 read_uint8(SDL_IOStream* stream) {
    char file_number_string[11];
    Uint8 string_index = 0;
    char file_char;
    bool in_comment = false;
    while(SDL_ReadIO(stream, &file_char, 1)) {
        if(file_char == '[') {
            in_comment = true;
        }
        else if(file_char == ']') {
            in_comment = false;
        }
        else if(!in_comment) {
            if(file_char != '\n' && file_char !=' ') {
                file_number_string[string_index] = file_char;
            }
            else {
                file_number_string[string_index] = '\0';
                return SDL_atoi(file_number_string);
            }
            string_index++;
        }
    }
    return 0;
}
Uint16 read_uint16(SDL_IOStream* stream) {
    char file_number_string[11];
    Uint16 string_index = 0;
    char file_char;
    bool in_comment = false;
    while(SDL_ReadIO(stream, &file_char, 1)) {
        if(file_char == '[') {
            in_comment = true;
        }
        else if(file_char == ']') {
            in_comment = false;
        }
        else if(!in_comment) {
            if(file_char != '\n' && file_char !=' ') {
                file_number_string[string_index] = file_char;
            }
            else {
                file_number_string[string_index] = '\0';
                return SDL_atoi(file_number_string);
            }
            string_index++;
        }
    }
    return 0;
}
Sint32 read_sint32(SDL_IOStream* stream) {
    char file_number_string[11];
    Uint8 string_index = 0;
    char file_char;
    bool in_comment = false;
    while(SDL_ReadIO(stream, &file_char, 1)) {
        if(file_char == '[') {
            in_comment = true;
        }
        else if(file_char == ']') {
            in_comment = false;
        }
        else if(!in_comment) {
            if(file_char != '\n' && file_char !=' ') {
                file_number_string[string_index] = file_char;
            }
            else {
                file_number_string[string_index] = '\0';
                return SDL_atoi(file_number_string);
            }
            string_index++;
        }
    }
    return 0;
}
void close_io(SDL_IOStream* stream1, SDL_IOStream* stream2) {
    SDL_CloseIO(stream1);
    SDL_CloseIO(stream2);
}