#include "game_state.h"

void GameState::register_keymaps() {
    key_mappings[MOVE_UP].keys.push(KEY_UP);
    key_mappings[MOVE_UP].keys.push(KEY_W);
    key_mappings[MOVE_DOWN].keys.push(KEY_DOWN);
    key_mappings[MOVE_DOWN].keys.push(KEY_S);
    key_mappings[MOVE_RIGHT].keys.push(KEY_RIGHT);
    key_mappings[MOVE_RIGHT].keys.push(KEY_D);
    key_mappings[MOVE_LEFT].keys.push(KEY_LEFT);
    key_mappings[MOVE_LEFT].keys.push(KEY_A);

    key_mappings[QUIT].keys.push(KEY_ESCAPE);

    key_mappings[MOUSE1].keys.push(KEY_MOUSE_LEFT);
    key_mappings[MOUSE2].keys.push(KEY_MOUSE_RIGHT);

    key_mappings[TOGGLE_FPS_CAP].keys.push(KEY_T);
}
