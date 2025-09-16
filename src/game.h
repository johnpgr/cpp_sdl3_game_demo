#pragma once

#include "SDL3/SDL_keycode.h"
#include "array.h"
#include "input.h"
#include "renderer.h"
#include "types.h"
#include "utils.h"
#include "math3d.h"

enum GameInputType : u8 {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    JUMP,

    MOUSE1,
    MOUSE2,

    QUIT,

    GAME_INPUT_COUNT
};

struct KeyMapping {
    Array<SDL_Keycode, 3> keys{};
    Array<u8, 2> mouse_buttons{};
};


struct GameState {
    bool quit{};
    bool fps_cap{};
    ivec2 player_position{};
    KeyMapping key_mappings[GAME_INPUT_COUNT]{};
};

static GameState* game_state{};

export void game_update(GameState* gs, InputState* is, RendererState* rs);
