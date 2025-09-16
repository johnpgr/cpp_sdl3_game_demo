#include "input.h"

bool InputState::key_pressed_this_frame(SDL_Keycode key_code) {
    Key key = input_state->keys[key_code];
    bool result = (key.is_down && key.half_transition_count == 1) ||
                  key.half_transition_count > 1;
    return result;
}

bool InputState::key_released_this_frame(SDL_Keycode key_code) {
    Key key = input_state->keys[key_code];
    bool result = (!key.is_down && key.half_transition_count == 1) ||
                  key.half_transition_count > 1;
    return result;
}

bool InputState::key_is_down(SDL_Keycode key_code) {
    return input_state->keys[key_code].is_down;
}

bool InputState::mouse_button_is_down(u8 button) {
    return input_state->mouse_buttons[button].is_down;
}
