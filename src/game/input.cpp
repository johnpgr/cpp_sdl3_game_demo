#include "game/input.h"
#include "SDL3/SDL_events.h"
#include "core/assert.h"
#include "gfx/renderer.h"

void Input::begin_frame() {
    for (i32 i = 0; i < KEY_COUNT; i++) {
        keys[i].just_pressed = false;
        keys[i].just_released = false;
        keys[i].half_transition_count = 0;
    }

    prev_mouse_pos = mouse_pos;
    prev_mouse_pos_world = mouse_pos_world;
}

void Input::process_key_event(SDL_KeyboardEvent* key_event) {
    SDL_Scancode scancode = key_event->scancode;
    if (scancode < 0 || scancode >= (SDL_Scancode)KEY_COUNT) return;

    Key* key = &input->keys[(KeyCodeId)scancode];
    bool was_down = key->is_down;
    key->is_down = (key_event->type == SDL_EVENT_KEY_DOWN);

    if (key->is_down != was_down) {
        key->half_transition_count++;
        key->just_pressed = key->is_down && !was_down;
        key->just_released = !key->is_down && was_down;
    }
}

void Input::process_mouse_motion(SDL_MouseMotionEvent* motion_event) {
    DEBUG_ASSERT(
        renderer != nullptr,
        "renderer is null in process_mouse_motion()"
    );
    input->mouse_pos = ivec2(motion_event->x, motion_event->y);
    input->rel_mouse = ivec2(motion_event->xrel, motion_event->yrel);

    input->mouse_pos_world = screen_to_world(input->mouse_pos);
    input->rel_mouse_world =
        input->mouse_pos_world - input->prev_mouse_pos_world;
}

void Input::process_mouse_button_event(SDL_MouseButtonEvent* button_event) {
    u8 button = button_event->button;
    if (button > 3) return;

    Key* mouse_button = &input->keys[KEY_MOUSE_LEFT + button - 1];
    bool was_down = mouse_button->is_down;
    mouse_button->is_down = (button_event->type == SDL_EVENT_MOUSE_BUTTON_DOWN);

    if (mouse_button->is_down != was_down) {
        mouse_button->half_transition_count++;
        mouse_button->just_pressed = mouse_button->is_down && !was_down;
        mouse_button->just_released = !mouse_button->is_down && was_down;
    }
}

bool Input::key_pressed_this_frame(KeyCodeId key_id) {
    Key key = input->keys[key_id];
    bool result = (key.is_down && key.half_transition_count == 1) ||
                  key.half_transition_count > 1;
    return result;
}

bool Input::key_released_this_frame(KeyCodeId key_id) {
    Key key = input->keys[key_id];
    bool result = (!key.is_down && key.half_transition_count == 1) ||
                  key.half_transition_count > 1;
    return result;
}

bool Input::key_is_down(KeyCodeId key_id) {
    return input->keys[key_id].is_down;
}
