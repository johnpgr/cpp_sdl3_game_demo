#pragma once

#include "SDL3/SDL_events.h"
#include "arena.h"
#include "math3d.h"
#include "types.h"
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_scancode.h>

// clang-format off
enum KeyCodeId {
    KEY_A              = SDL_SCANCODE_A,
    KEY_B              = SDL_SCANCODE_B,
    KEY_C              = SDL_SCANCODE_C,
    KEY_D              = SDL_SCANCODE_D,
    KEY_E              = SDL_SCANCODE_E,
    KEY_F              = SDL_SCANCODE_F,
    KEY_G              = SDL_SCANCODE_G,
    KEY_H              = SDL_SCANCODE_H,
    KEY_I              = SDL_SCANCODE_I,
    KEY_J              = SDL_SCANCODE_J,
    KEY_K              = SDL_SCANCODE_K,
    KEY_L              = SDL_SCANCODE_L,
    KEY_M              = SDL_SCANCODE_M,
    KEY_N              = SDL_SCANCODE_N,
    KEY_O              = SDL_SCANCODE_O,
    KEY_P              = SDL_SCANCODE_P,
    KEY_Q              = SDL_SCANCODE_Q,
    KEY_R              = SDL_SCANCODE_R,
    KEY_S              = SDL_SCANCODE_S,
    KEY_T              = SDL_SCANCODE_T,
    KEY_U              = SDL_SCANCODE_U,
    KEY_V              = SDL_SCANCODE_V,
    KEY_W              = SDL_SCANCODE_W,
    KEY_X              = SDL_SCANCODE_X,
    KEY_Y              = SDL_SCANCODE_Y,
    KEY_Z              = SDL_SCANCODE_Z,

    KEY_0              = SDL_SCANCODE_0,
    KEY_1              = SDL_SCANCODE_1,
    KEY_2              = SDL_SCANCODE_2,
    KEY_3              = SDL_SCANCODE_3,
    KEY_4              = SDL_SCANCODE_4,
    KEY_5              = SDL_SCANCODE_5,
    KEY_6              = SDL_SCANCODE_6,
    KEY_7              = SDL_SCANCODE_7,
    KEY_8              = SDL_SCANCODE_8,
    KEY_9              = SDL_SCANCODE_9,

    KEY_SPACE          = SDL_SCANCODE_SPACE,
    KEY_TICK           = SDL_SCANCODE_GRAVE,
    KEY_MINUS          = SDL_SCANCODE_MINUS,
    KEY_EQUAL          = SDL_SCANCODE_EQUALS,
    KEY_LEFT_BRACKET   = SDL_SCANCODE_LEFTBRACKET,
    KEY_RIGHT_BRACKET  = SDL_SCANCODE_RIGHTBRACKET,
    KEY_SEMICOLON      = SDL_SCANCODE_SEMICOLON,
    KEY_QUOTE          = SDL_SCANCODE_APOSTROPHE,
    KEY_COMMA          = SDL_SCANCODE_COMMA,
    KEY_PERIOD         = SDL_SCANCODE_PERIOD,
    KEY_FORWARD_SLASH  = SDL_SCANCODE_SLASH,
    KEY_BACKWARD_SLASH = SDL_SCANCODE_BACKSLASH,
    KEY_TAB            = SDL_SCANCODE_TAB,
    KEY_ESCAPE         = SDL_SCANCODE_ESCAPE,
    KEY_PAUSE          = SDL_SCANCODE_PAUSE,
    KEY_UP             = SDL_SCANCODE_UP,
    KEY_DOWN           = SDL_SCANCODE_DOWN,
    KEY_LEFT           = SDL_SCANCODE_LEFT,
    KEY_RIGHT          = SDL_SCANCODE_RIGHT,
    KEY_BACKSPACE      = SDL_SCANCODE_BACKSPACE,
    KEY_RETURN         = SDL_SCANCODE_RETURN,
    KEY_DELETE         = SDL_SCANCODE_DELETE,
    KEY_INSERT         = SDL_SCANCODE_INSERT,
    KEY_HOME           = SDL_SCANCODE_HOME,
    KEY_END            = SDL_SCANCODE_END,
    KEY_PAGE_UP        = SDL_SCANCODE_PAGEUP,
    KEY_PAGE_DOWN      = SDL_SCANCODE_PAGEDOWN,
    KEY_CAPS_LOCK      = SDL_SCANCODE_CAPSLOCK,
    KEY_NUM_LOCK       = SDL_SCANCODE_NUMLOCKCLEAR,
    KEY_SCROLL_LOCK    = SDL_SCANCODE_SCROLLLOCK,
    KEY_MENU           = SDL_SCANCODE_MENU,
    KEY_SHIFT          = SDL_SCANCODE_LSHIFT,
    KEY_CONTROL        = SDL_SCANCODE_LCTRL,
    KEY_ALT            = SDL_SCANCODE_LALT,
    KEY_COMMAND        = SDL_SCANCODE_LGUI,

    KEY_F1             = SDL_SCANCODE_F1,
    KEY_F2             = SDL_SCANCODE_F2,
    KEY_F3             = SDL_SCANCODE_F3,
    KEY_F4             = SDL_SCANCODE_F4,
    KEY_F5             = SDL_SCANCODE_F5,
    KEY_F6             = SDL_SCANCODE_F6,
    KEY_F7             = SDL_SCANCODE_F7,
    KEY_F8             = SDL_SCANCODE_F8,
    KEY_F9             = SDL_SCANCODE_F9,
    KEY_F10            = SDL_SCANCODE_F10,
    KEY_F11            = SDL_SCANCODE_F11,
    KEY_F12            = SDL_SCANCODE_F12,

    KEY_NUMPAD_0       = SDL_SCANCODE_KP_0,
    KEY_NUMPAD_1       = SDL_SCANCODE_KP_1,
    KEY_NUMPAD_2       = SDL_SCANCODE_KP_2,
    KEY_NUMPAD_3       = SDL_SCANCODE_KP_3,
    KEY_NUMPAD_4       = SDL_SCANCODE_KP_4,
    KEY_NUMPAD_5       = SDL_SCANCODE_KP_5,
    KEY_NUMPAD_6       = SDL_SCANCODE_KP_6,
    KEY_NUMPAD_7       = SDL_SCANCODE_KP_7,
    KEY_NUMPAD_8       = SDL_SCANCODE_KP_8,
    KEY_NUMPAD_9       = SDL_SCANCODE_KP_9,

    KEY_NUMPAD_STAR    = SDL_SCANCODE_KP_MULTIPLY,
    KEY_NUMPAD_PLUS    = SDL_SCANCODE_KP_PLUS,
    KEY_NUMPAD_MINUS   = SDL_SCANCODE_KP_MINUS,
    KEY_NUMPAD_DOT     = SDL_SCANCODE_KP_DECIMAL,
    KEY_NUMPAD_SLASH   = SDL_SCANCODE_KP_DIVIDE,

    KEY_MOUSE_LEFT,
    KEY_MOUSE_MIDDLE,
    KEY_MOUSE_RIGHT,
    KEY_MOUSE4,
    KEY_MOUSE5,

    KEY_COUNT          = SDL_SCANCODE_COUNT,
};
// clang-format on

struct Key {
    bool is_down{};
    bool just_pressed{};
    bool just_released{};
    u8 half_transition_count{};
};

struct Input {
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

    void begin_frame();
    void process_key_event(SDL_KeyboardEvent* key_event);
    void process_mouse_motion(SDL_MouseMotionEvent* motion_event);
    void process_mouse_button_event(SDL_MouseButtonEvent* button_event);
    bool key_pressed_this_frame(KeyCodeId scan_code);
    bool key_released_this_frame(KeyCodeId scan_code);
    bool key_is_down(KeyCodeId scan_code);
};

static Input* input{};

void init_input(Arena* arena);
