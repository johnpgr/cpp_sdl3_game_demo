#include "arena.h"
#include "consts.h"
#include "input.h"
#include "renderer.h"
#include "types.h"
#include "utils.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static u64 fps_count = 0;
static bool running = true;

static void update_input_begin_frame() {
    for (i32 i = 0; i < KEY_COUNT; i++) {
        input_state->keys[i].just_pressed = false;
        input_state->keys[i].just_released = false;
        input_state->keys[i].half_transition_count = 0;
    }
    input_state->prev_mouse_pos = input_state->mouse_pos;
    input_state->prev_mouse_pos_world = input_state->mouse_pos_world;
}

static void process_key_event(SDL_KeyboardEvent* key_event) {
    SDL_Keycode keycode = key_event->key;
    if (keycode < 0 || keycode >= KEY_COUNT) return;

    Key* key = &input_state->keys[keycode];
    bool was_down = key->is_down;
    key->is_down = (key_event->type == SDL_EVENT_KEY_DOWN);

    if (key->is_down != was_down) {
        key->half_transition_count++;
        key->just_pressed = key->is_down && !was_down;
        key->just_released = !key->is_down && was_down;
    }
}

static void process_mouse_motion(SDL_MouseMotionEvent* motion_event) {
    input_state->mouse_pos = ivec2(motion_event->x, motion_event->y);
    input_state->rel_mouse = ivec2(motion_event->xrel, motion_event->yrel);

    if (renderer_state != nullptr) {
        input_state->mouse_pos_world = screen_to_world(input_state->mouse_pos);
        input_state->rel_mouse_world =
            input_state->mouse_pos_world - input_state->prev_mouse_pos_world;
    }
}

static void update_input_end_frame() {}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    Arena transient_storage(MB(128));
    Arena permanent_storage(MB(64));

    defer {
        transient_storage.destroy();
        permanent_storage.destroy();
    };

    SDL_Window* window =
        SDL_CreateWindow("Imperfections", WIDTH * 3, HEIGHT * 3, 0);
    input_state = InputState::create(&permanent_storage, WIDTH * 3, HEIGHT * 3);

    SDL_Event event{};
    while (running) {
        update_input_end_frame();

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP:
                    process_key_event(&event.key);
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    process_mouse_motion(&event.motion);
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    input_state->screen_size =
                        ivec2(event.window.data1, event.window.data2);
                    break;
            }
        }
        update_input_end_frame();
    }
}
