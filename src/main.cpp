#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_timer.h"
#include "arena.cpp"
#include "array.cpp"
#include "assert.cpp"
#include "file.cpp"
#include "game.h"
#include "input.cpp"
#include "input.h"
#include "math3d.cpp"
#include "renderer.cpp"
#include "sprite_atlas.cpp"
#include "types.h"
#include "utils.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

typedef void GameUpdateFn(
    GameState*,
    InputState*,
    SpriteAtlas*,
    RendererState*
);
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

        while (!copy_file(
            transient_storage,
            DYNLIB("libgame"),
            DYNLIB("libgame_load")
        )) {
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

void game_update(
    GameState* gs,
    InputState* is,
    SpriteAtlas* sa,
    RendererState* rs
) {
    DEBUG_ASSERT(game_update_ptr != nullptr, "game_update_ptr is null");
    game_update_ptr(gs, is, sa, rs);
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

void poll_events(SDL_Event* event) {
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_EVENT_QUIT:
                game_state->quit = true;
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                input_state->process_key_event(&event->key);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                input_state->process_mouse_button_event(&event->button);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                input_state->process_mouse_motion(&event->motion);
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                input_state->screen_size =
                    ivec2(event->window.data1, event->window.data2);
                break;
        }
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    Arena transient_storage(MB(128));
    Arena permanent_storage(MB(64));

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    defer {
        transient_storage.destroy();
        permanent_storage.destroy();

        SDL_Quit();
    };

    init_game_state(&permanent_storage);
    init_input_state(&permanent_storage);

    if (!init_renderer_state(&permanent_storage)) {
        SDL_Log("Failed to initialize renderer_state");
        return EXIT_FAILURE;
    }

    if (!init_sprite_atlas(&permanent_storage, "TEXTURE_ATLAS.png")) {
        SDL_Log("Failed to initialize sprite_atlas");
        return EXIT_FAILURE;
    }

    register_sprites();

    SDL_ShowWindow(renderer_state->window);

    const u64 NANOS_PER_UPDATE = NANOS_PER_SEC / FPS;
    u64 accumulator = 0;
    u64 last_time = SDL_GetTicksNS();

    while (!game_state->quit) {
        u64 current_time = SDL_GetTicksNS();
        u64 delta_time = current_time - last_time;
        last_time = current_time;
        accumulator += delta_time;

        reload_game_dll(&transient_storage);

        SDL_Event event{};
        poll_events(&event);

        while (accumulator >= NANOS_PER_UPDATE) {
            input_state->begin_frame();
            game_update(game_state, input_state, sprite_atlas, renderer_state);
            renderer_state->render();

            accumulator -= NANOS_PER_UPDATE;
        }

        transient_storage.clear();
    }

    return EXIT_SUCCESS;
}
