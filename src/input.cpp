#include "input.h"

bool InputState::key_pressed_this_frame(KeyCodeId key_id) {
    Key key = input_state->keys[key_id];
    bool result = (key.is_down && key.half_transition_count == 1) ||
                  key.half_transition_count > 1;
    return result;
}

bool InputState::key_released_this_frame(KeyCodeId key_id) {
    Key key = input_state->keys[key_id];
    bool result = (!key.is_down && key.half_transition_count == 1) ||
                  key.half_transition_count > 1;
    return result;
}

bool InputState::key_is_down(KeyCodeId key_id) {
    return input_state->keys[key_id].is_down;
}
