#pragma once

#include "array.h"
#include "assets.h"
#include "math3d.h"
#include "types.h"
#include "arena.h"
#include <SDL3/SDL.h>

struct Camera2d {
    f32 zoom{1.0};
    vec2 dimensions{};
    vec2 position{};
};

struct Transform {
    ivec2 atlas_offset{};
    ivec2 sprite_size{};
    vec2 position{};
    vec2 size{};
};

struct RendererState {
    SDL_Window* window{};
    SDL_GPUDevice* device{};

    Camera2d game_camera{};
    Camera2d ui_camera{};
    Array<Transform, 1000> transforms{};
};

static RendererState* renderer_state{};

ivec2 screen_to_world(ivec2 screen_pos);
void draw_sprite(SpriteId sprite_id, vec2 pos);
void draw_sprite(SpriteId sprite_id, ivec2 pos);

SDL_Surface* load_image(const char* image_filename, i32 desired_channels = 4);

SDL_GPUShader* load_shader(
    Arena* arena,
    SDL_GPUDevice* device,
    const char* shader_name,
    u32 num_samplers,
    u32 num_uniform_buffers,
    u32 num_storage_buffers,
    u32 num_storage_textures
);
SDL_GPUTexture* gpu_texture_from_surface();
