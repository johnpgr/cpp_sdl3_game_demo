#include "game/game.h"
#include "core/types.h"
#include "gfx/sprite.h"
#include "gfx/sprite_atlas.cpp"
#include "core/arena.cpp"
#include "core/array.cpp"
#include "core/assert.cpp"
#include "core/file.cpp"
#include "game/input.cpp"
#include "core/math3d.cpp"
#include "gfx/renderer.cpp"
#include <SDL3/SDL.h>

static bool just_pressed(GameInputType type) {
    KeyMapping mapping = game_state->key_mappings[type];

    for (usize idx = 0; idx < mapping.keys.size; idx++) {
        if (input->keys[mapping.keys[idx]].just_pressed) {
            return true;
        }
    }

    return false;
}

static bool is_down(GameInputType type) {
    KeyMapping mapping = game_state->key_mappings[type];

    for (usize idx = 0; idx < mapping.keys.size; idx++) {
        if (input->keys[mapping.keys[idx]].is_down) {
            return true;
        }
    }

    return false;
}

EXPORT_FN void game_update(GameState* gs, Input* is, SpriteAtlas* sa, Renderer* rs) {
    if (gs != game_state) {
        game_state = gs;
        input = is;
        sprite_atlas = sa;
        renderer = rs;
    }

    renderer->draw_sprite(SPRITE_DICE, game_state->player_position);
    renderer->draw_text("Hello, World!", vec2(0, 0), vec4(1.0f, 1.0f, 1.0f, 1.0f));

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
    if (is_down(MOUSE1)) {
        ivec2 world_pos = screen_to_world(input->mouse_pos);
        renderer->draw_sprite(SPRITE_WHITE, world_pos, vec2(8));
    }
    if (is_down(MOUSE2)) {
    }
}
