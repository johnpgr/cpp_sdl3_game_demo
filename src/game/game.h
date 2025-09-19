#pragma once

#include "core/array.h"
#include "game/input.h"
#include "core/math3d.h"
#include "gfx/renderer.h"
#include "gfx/sprite_atlas.h"
#include "core/utils.h"
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
};

static GameState* game_state{};

EXPORT_FN void game_update(GameState* gs, Input* is, SpriteAtlas* sa, Renderer* r);
