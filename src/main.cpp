#include "arena.cpp"
#include "array.cpp"
#include "assert.cpp"
#include "assets.cpp"
#include "consts.h"
#include "file.cpp"
#include "game.h"
#include "input.cpp"
#include "input.h"
#include "math3d.cpp"
#include "renderer.cpp"
#include "types.h"
#include "utils.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

typedef void GameUpdateFn(GameState*, InputState*, RendererState*);
static GameUpdateFn* game_update_ptr;

static void reload_game_dll(Arena* transient_storage) {
    static SDL_SharedObject* game_dll;
    static u64 game_dll_timestamp;

    u64 current_dll_timestamp = file_get_timestamp(DYNLIB("libgame"));

    if (current_dll_timestamp > game_dll_timestamp) {
        if (game_dll) {
            SDL_UnloadObject(game_dll);
            game_dll = nullptr;
            SDL_Log("Unloaded old game dynlib");
        }

        while (
            !copy_file(transient_storage, DYNLIB("libgame"), DYNLIB("libgame_load"))
        ) {
            SDL_Delay(10);
        }

        game_dll = SDL_LoadObject(DYNLIB("libgame_load"));
        DEBUG_ASSERT(game_dll != nullptr, "Failed to load game dynlib");

        game_update_ptr =
            (GameUpdateFn*)SDL_LoadFunction(game_dll, "game_update");
        DEBUG_ASSERT(
            game_update_ptr != nullptr,
            "Failed to load game_update function"
        );
        game_dll_timestamp = current_dll_timestamp;
    }
}

static void init_game_state(Arena* arena) {
    game_state = arena->push_struct<GameState>();

    game_state->key_mappings[MOVE_UP].keys.push(KEY_UP);
    game_state->key_mappings[MOVE_UP].keys.push(KEY_W);
    game_state->key_mappings[MOVE_DOWN].keys.push(KEY_DOWN);
    game_state->key_mappings[MOVE_DOWN].keys.push(KEY_S);
    game_state->key_mappings[MOVE_RIGHT].keys.push(KEY_RIGHT);
    game_state->key_mappings[MOVE_RIGHT].keys.push(KEY_D);
    game_state->key_mappings[MOVE_LEFT].keys.push(KEY_LEFT);
    game_state->key_mappings[MOVE_LEFT].keys.push(KEY_A);

    game_state->key_mappings[QUIT].keys.push(KEY_ESCAPE);

    game_state->key_mappings[MOUSE1].keys.push(KEY_MOUSE_LEFT);
    game_state->key_mappings[MOUSE2].keys.push(KEY_MOUSE_RIGHT);
}

static void init_input_state(Arena* arena) {
    input_state = arena->push_struct<InputState>();
    input_state->screen_size.x = INITIAL_WINDOW_WIDTH;
    input_state->screen_size.y = INITIAL_WINDOW_HEIGHT;
}

static void init_renderer_state(Arena* arena) {
    renderer_state = arena->push_struct<RendererState>();
    renderer_state->game_camera.dimensions[0] = WIDTH;
    renderer_state->game_camera.dimensions[1] = HEIGHT;
    renderer_state->game_camera.position[0] = 160;
    renderer_state->game_camera.position[1] = -90;
    renderer_state->ui_camera.dimensions[0] = WIDTH;
    renderer_state->ui_camera.dimensions[0] = HEIGHT;
    renderer_state->ui_camera.position[0] = 160;
    renderer_state->ui_camera.position[0] = -90;
}

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
    SDL_Scancode scancode = key_event->scancode;
    if (scancode < 0 || scancode >= (SDL_Scancode)KEY_COUNT) return;

    Key* key = &input_state->keys[(KeyCodeId)scancode];
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

static void process_mouse_button_event(SDL_MouseButtonEvent* button_event) {
    u8 button = button_event->button;
    if (button > 3) return;

    Key* mouse_button = &input_state->keys[KEY_MOUSE_LEFT + button - 1];
    bool was_down = mouse_button->is_down;
    mouse_button->is_down = (button_event->type == SDL_EVENT_MOUSE_BUTTON_DOWN);

    if (mouse_button->is_down != was_down) {
        mouse_button->half_transition_count++;
        mouse_button->just_pressed = mouse_button->is_down && !was_down;
        mouse_button->just_released = !mouse_button->is_down && was_down;
    }
}

void game_update(GameState* gs, InputState* is, RendererState* rs) {
    DEBUG_ASSERT(game_update_ptr != nullptr, "game_update_ptr is null");
    game_update_ptr(gs, is, rs);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    Arena transient_storage(MB(128));
    Arena permanent_storage(MB(64));

    defer {
        transient_storage.destroy();
        permanent_storage.destroy();
    };

    init_game_state(&permanent_storage);
    init_input_state(&permanent_storage);
    init_renderer_state(&permanent_storage);

    SDL_Window* window = SDL_CreateWindow(
        "The game",
        INITIAL_WINDOW_WIDTH,
        INITIAL_WINDOW_HEIGHT,
        0
    );
    defer {
        SDL_DestroyWindow(window);
    };

    SDL_Event event{};
    while (!game_state->quit) {
        reload_game_dll(&transient_storage);
        update_input_begin_frame();

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    game_state->quit = true;
                    break;
                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP:
                    process_key_event(&event.key);
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    process_mouse_button_event(&event.button);
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

        game_update(game_state, input_state, renderer_state);

        transient_storage.clear();
    }

    return EXIT_SUCCESS;
}
