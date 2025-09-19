#include "game/game.h"
#include "game/input.h"
#include "core/types.h"
#include "core/utils.h"
#include "core/arena.cpp"
#include "core/array.cpp"
#include "core/assert.cpp"
#include "core/file.cpp"
#include "core/math3d.cpp"
#include "game/input.cpp"
#include "gfx/renderer.cpp"
#include "gfx/sprite_atlas.cpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

// FPS tracking
static f32 fps = 0.0f;
static f32 frame_time_accumulator = 0.0f;
static i32 frame_count = 0;
static f32 last_title_update_time = 0.0f;

typedef void GameUpdateFn(GameState*, Input*, SpriteAtlas*, Renderer*);
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
        if(!game_dll) {
            SDL_Log("Failed to load game dynlib: %s", SDL_GetError());
            return;
        }

        game_update_ptr =
            (GameUpdateFn*)SDL_LoadFunction(game_dll, "game_update");
        if (!game_update_ptr) {
            SDL_Log("Failed to load game_update function: %s", SDL_GetError());
            return;
        }
        game_dll_timestamp = current_dll_timestamp;
    }
}

void game_update(GameState* gs, Input* is, SpriteAtlas* sa, Renderer* rs) {
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

    game_state->key_mappings[TOGGLE_FPS_CAP].keys.push(KEY_T);
}

void poll_events() {
    SDL_Event event{};

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                game_state->quit = true;
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                input->process_key_event(&event.key);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                input->process_mouse_button_event(&event.button);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                input->process_mouse_motion(&event.motion);
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                input->screen_size =
                    ivec2(event.window.data1, event.window.data2);
                break;
        }
    }
}

static void update_window_title(f32 current_time) {
    // Update title every 0.5 seconds
    if (current_time - last_title_update_time < 0.5f) {
        return;
    }
    last_title_update_time = current_time;

    // Format title with FPS and memory usage
    char title[256];
    SDL_snprintf(title, sizeof(title), "FPS: %.1f", fps);

    SDL_SetWindowTitle(renderer->window, title);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    Arena transient_storage(MB(32));
    Arena permanent_storage(MB(64));

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();

    defer {
        if (sprite_atlas) sprite_atlas->destroy();
        if (renderer) renderer->destroy();
        transient_storage.destroy();
        permanent_storage.destroy();

        TTF_Init();
        SDL_Quit();
    };

    init_game_state(&permanent_storage);
    init_input(&permanent_storage);

    if (!init_renderer(&permanent_storage)) {
        SDL_Log("Failed to initialize renderer");
        return EXIT_FAILURE;
    }

    if (!init_text_renderer(&permanent_storage, "assets/fonts/dejavu.ttf")) {
        SDL_Log("Failed to initialize text_renderer");
        return EXIT_FAILURE;
    }

    if (!init_sprite_atlas(&permanent_storage, "TEXTURE_ATLAS.png")) {
        SDL_Log("Failed to initialize sprite_atlas");
        return EXIT_FAILURE;
    }

    register_sprites();

    SDL_ShowWindow(renderer->window);

    u64 last_time = SDL_GetPerformanceCounter();
    u64 frequency = SDL_GetPerformanceFrequency();
    const f32 target_frame_time = 1.0f / (f32)FPS;

    while (!game_state->quit) {
        u64 frame_start = SDL_GetPerformanceCounter();
        f32 delta_time = (f32)(frame_start - last_time) / (f32)frequency;
        last_time = frame_start;

        // Calculate FPS
        frame_time_accumulator += delta_time;
        frame_count++;

        // Update FPS every second
        if (frame_time_accumulator >= 1.0f) {
            fps = (f32)frame_count / frame_time_accumulator;
            frame_time_accumulator = 0.0f;
            frame_count = 0;
        }

        f32 current_time_seconds = (f32)frame_start / frequency;
        update_window_title(current_time_seconds);

        reload_game_dll(&transient_storage);

        input->begin_frame();

        poll_events();

        game_update(game_state, input, sprite_atlas, renderer);
        renderer->render();

        if (game_state->fps_cap) {
            u64 frame_end = SDL_GetPerformanceCounter();
            f32 frame_elapsed = (f32)(frame_end - frame_start) / (f32)frequency;
            f32 remaining = target_frame_time - frame_elapsed;
            if (remaining > 0.0f) {
                Uint32 ms = (Uint32)(remaining * 1000.0f);
                if (ms > 1) {
                    SDL_Delay(ms - 1); // coarse sleep
                }
                // busy-wait until target_frame_time reached
                while ((f32)(SDL_GetPerformanceCounter() - frame_start) /
                           (f32)frequency <
                       target_frame_time) {
                }
            }
        }

        transient_storage.clear();
    }

    return EXIT_SUCCESS;
}
