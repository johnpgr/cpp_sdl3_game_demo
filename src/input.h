#pragma once

#include "arena.h"
#include "types.h"
#include "math3d.h"
#include <SDL3/SDL_keycode.h>

constexpr int KEY_COUNT = 512;

struct Key {
    bool is_down{};
    bool just_pressed{};
    bool just_released{};
    u8 half_transition_count{};
};

struct InputState {
    ivec2 screen_size{};

    // Screen
    ivec2 prev_mouse_pos{};
    ivec2 mouse_pos{};
    ivec2 rel_mouse{};

    // World
    ivec2 prev_mouse_pos_world{};
    ivec2 mouse_pos_world{};
    ivec2 rel_mouse_world{};

    Key keys[KEY_COUNT]{};

    static InputState* create(Arena* arena, i32 screen_width, i32 screen_height) {
        InputState* state = arena->push_struct<InputState>();
        state->screen_size.x = screen_width;
        state->screen_size.y = screen_height;
        return state;
    }
};

static InputState* input_state;

static bool key_pressed_this_frame(SDL_Keycode key_code) {
    Key key = input_state->keys[key_code];
    bool result = (key.is_down && key.half_transition_count == 1) ||
                  key.half_transition_count > 1;
    return result;
}

static bool key_released_this_frame(SDL_Keycode key_code) {
    Key key = input_state->keys[key_code];
    bool result = (!key.is_down && key.half_transition_count == 1) ||
                  key.half_transition_count > 1;
    return result;
}

bool key_is_down(SDL_Keycode key_code) { return input_state->keys[key_code].is_down; }
