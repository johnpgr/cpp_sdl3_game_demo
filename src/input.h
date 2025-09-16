#pragma once

#include "math3d.h"
#include "types.h"
#include <SDL3/SDL_keycode.h>

#define KEY_COUNT 512
#define MOUSE_BUTTON_COUNT 8

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
    Key mouse_buttons[MOUSE_BUTTON_COUNT]{};

    bool key_pressed_this_frame(SDL_Keycode key_code);
    bool key_released_this_frame(SDL_Keycode key_code);
    bool key_is_down(SDL_Keycode key_code);
    bool mouse_button_is_down(u8 button);
};

static InputState* input_state{};
