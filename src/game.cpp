#include "game.h"
#include "arena.cpp"
#include "array.cpp"
#include "assert.cpp"
#include "assets.cpp"
#include "consts.h"
#include "file.cpp"
#include "input.cpp"
#include "math3d.cpp"
#include "renderer.cpp"
#include "types.h"
#include "utils.h"
#include <SDL3/SDL.h>

static bool just_pressed(GameInputType type) {
    KeyMapping mapping = game_state->key_mappings[type];

    if (type == MOUSE1 || type == MOUSE2) {
        for (usize idx = 0; idx < mapping.mouse_buttons.size; idx++) {
            if (input_state->mouse_buttons[mapping.keys[idx]].just_pressed) {
                return true;
            }
        }
    }

    for (usize idx = 0; idx < mapping.keys.size; idx++) {
        if (input_state->keys[mapping.keys[idx]].just_pressed) {
            return true;
        }
    }

    return false;
}

static bool is_down(GameInputType type) {
    KeyMapping mapping = game_state->key_mappings[type];

    if (type == MOUSE1 || type == MOUSE2) {
        for (usize idx = 0; idx < mapping.mouse_buttons.size; idx++) {
            if (input_state->mouse_buttons[mapping.keys[idx]].is_down) {
                return true;
            }
        }
    }

    for (usize idx = 0; idx < mapping.keys.size; idx++) {
        if (input_state->keys[mapping.keys[idx]].is_down) {
            return true;
        }
    }

    return false;
}

export void game_update(GameState* gs, InputState* is, RendererState* rs) {
    if (gs != game_state) {
        game_state = gs;
        input_state = is;
        renderer_state = rs;
    }

    if (is_down(QUIT)) {
        game_state->quit = true;
    }
    if (is_down(MOVE_LEFT)) {
        game_state->player_position.x -= 1;
        SDL_Log("MOVE_LEFT");
    }
    if (is_down(MOVE_RIGHT)) {
        game_state->player_position.x += 1;
        SDL_Log("MOVE_RIGHT");
    }
    if (is_down(MOVE_UP)) {
        game_state->player_position.y -= 1;
        SDL_Log("MOVE_UP");
    }
    if (is_down(MOVE_DOWN)) {
        game_state->player_position.y += 1;
        SDL_Log("MOVE_DOWN");
    }
    if (is_down(MOUSE1)) {
        SDL_Log("MOUSE1");
    }
    if (is_down(MOUSE2)) {
        SDL_Log("MOUSE2");
    }
}
