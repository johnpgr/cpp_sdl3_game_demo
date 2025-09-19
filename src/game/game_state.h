#pragma once

#include "core/array.h"
#include "core/math3d.h"
#include "game/input.h"
#include <SDL3/SDL_scancode.h>

enum GameInputType {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    JUMP,

    MOUSE1,
    MOUSE2,

    QUIT,

    TOGGLE_FPS_CAP,

    GAME_INPUT_COUNT
};

struct KeyMapping {
    Array<KeyCodeId, 3> keys{};
};

struct GameState {
    bool quit{};
    bool fps_cap{true};
    ivec2 player_position{};
    KeyMapping key_mappings[GAME_INPUT_COUNT]{};

    void register_keymaps();
};

static GameState* game_state{};
