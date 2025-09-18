#include "game.h"
#include "arena.cpp"
#include "array.cpp"
#include "assert.cpp"
#include "assets.h"
#include "file.cpp"
#include "input.cpp"
#include "math3d.cpp"
#include "renderer.cpp"
#include "sprite_atlas.cpp"
#include "types.h"
#include <SDL3/SDL.h>

static bool just_pressed(GameInputType type) {
    KeyMapping mapping = game_state->key_mappings[type];

    for (usize idx = 0; idx < mapping.keys.size; idx++) {
        if (input_state->keys[mapping.keys[idx]].just_pressed) {
            return true;
        }
    }

    return false;
}

static bool is_down(GameInputType type) {
    KeyMapping mapping = game_state->key_mappings[type];

    for (usize idx = 0; idx < mapping.keys.size; idx++) {
        if (input_state->keys[mapping.keys[idx]].is_down) {
            return true;
        }
    }

    return false;
}

EXPORT_FN void game_update(GameState* gs, InputState* is, SpriteAtlas* sa, RendererState* rs) {
    if (gs != game_state) {
        game_state = gs;
        input_state = is;
        sprite_atlas = sa;
        renderer_state = rs;
    }

    draw_sprite(SPRITE_DICE, game_state->player_position);

    if (just_pressed(TOGGLE_FPS_CAP)) {
        game_state->fps_cap = !game_state->fps_cap;
    }

    if (is_down(QUIT)) {
        game_state->quit = true;
    }
    if (is_down(MOVE_LEFT)) {
        game_state->player_position.x -= 1;
    }
    if (is_down(MOVE_RIGHT)) {
        game_state->player_position.x += 1;
    }
    if (is_down(MOVE_UP)) {
        game_state->player_position.y -= 1;
    }
    if (is_down(MOVE_DOWN)) {
        game_state->player_position.y += 1;
    }
    if (just_pressed(MOUSE1)) {
        ivec2 world_pos = screen_to_world(input_state->mouse_pos);
        draw_sprite(SPRITE_WHITE, world_pos, vec2(8));
    }
    if (just_pressed(MOUSE2)) {
    }
}
