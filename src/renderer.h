#pragma once

#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_video.h"
#include "arena.h"
#include "array.h"
#include "math3d.h"
#include "types.h"

struct Camera2d {
    f32 zoom{1.0};
    vec2 dimensions{};
    vec2 position{};
};

struct Transform {
    vec2 pos{};
    vec2 size{};
    vec2 uv_min{}; // Normalized UV coordinates from sprite atlas
    vec2 uv_max{}; // Normalized UV coordinates from sprite atlas
};

struct RendererState {
    SDL_Window* window{};
    SDL_GPUDevice* device{};
    SDL_GPUGraphicsPipeline* pipeline{};
    SDL_GPUBuffer* transform_buffer{};
    SDL_GPUBuffer* quad_vertex_buffer{};
    SDL_GPUBuffer* quad_index_buffer{};

    Camera2d game_camera{};
    Camera2d ui_camera{};
    Array<Transform, 1000> transforms{};

    void destroy();
    void render();
};

static RendererState* renderer_state{};

bool init_renderer_state(Arena* arena);

ivec2 screen_to_world(ivec2 screen_pos);

SDL_Surface* load_image(const char* image_filename, i32 desired_channels = 4);

struct ShaderProps {
    u32 num_samplers{};
    u32 num_uniform_buffers{};
    u32 num_storage_buffers{};
    u32 num_storage_textures{};
};

SDL_GPUShader* load_shader(
    const char* shader_name,
    SDL_GPUDevice* device,
    SDL_GPUShaderStage shader_stage,
    ShaderProps shader_props
);

SDL_GPUTexture* gpu_texture_from_surface(SDL_Surface* surface);
