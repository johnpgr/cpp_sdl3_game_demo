#pragma once

#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_video.h"
#include "SDL3_ttf/SDL_ttf.h"
#include "core/arena.h"
#include "core/array.h"
#include "core/math3d.h"
#include "core/types.h"
#include "game/consts.h"
#include "gfx/sprite.h"

// TODO: Need to find a better ideal number for these
#define MAX_SPRITES 5000
#define MAX_TEXT_VERTICES 5000
#define MAX_TEXT_INDICES 5000

enum FontSize {
    FONTSIZE_EXTRASMALL,
    FONTSIZE_SMALL,
    FONTSIZE_MEDIUM,
    FONTSIZE_LARGE,
    FONTSIZE_EXTRALARGE,

    FONTSIZE_COUNT,
};

struct Camera2d {
    f32 zoom{1.0};
    vec2 dimensions{WIDTH, HEIGHT};
    vec2 position{160, -90};
};

struct SpriteVertex {
    vec2 pos{};
    vec2 size{};
    vec2 uv_min{}; // Normalized UV coordinates from sprite atlas
    vec2 uv_max{}; // Normalized UV coordinates from sprite atlas
};

struct TextVertex {
    vec3 pos{};
    vec4 color{};
    vec2 uv{};
};

struct QueuedText {
    char text[256];
    vec2 position;
    vec4 color;
    FontSize font_size;
};

// Text geometry data for batching text rendering
struct TextGeometryData {
    Array<TextVertex, MAX_TEXT_VERTICES> vertices{};
    Array<i32, MAX_TEXT_INDICES> indices{};

    void reset();
    void queue_text_sequence(
        TTF_GPUAtlasDrawSequence* sequence,
        vec4 color,
        vec2 offset = {0, 0}
    );
};

struct Renderer {
    SDL_Window* window{};
    SDL_GPUDevice* device{};

    // Sprite rendering
    SDL_GPUGraphicsPipeline* sprite_pipeline{};
    SDL_GPUBuffer* sprite_vertex_buffer{};
    SDL_GPUBuffer* sprite_quad_vertex_buffer{};
    SDL_GPUBuffer* sprite_quad_index_buffer{};
    SDL_GPUTransferBuffer* sprite_transfer_buffer{};

    // Text rendering
    SDL_GPUGraphicsPipeline* text_pipeline{};
    SDL_GPUBuffer* text_vertex_buffer{};
    SDL_GPUBuffer* text_index_buffer{};
    SDL_GPUTransferBuffer* text_transfer_buffer{};
    SDL_GPUSampler* text_sampler{};
    TTF_TextEngine* text_engine{};
    SDL_GPUTexture* text_atlas_texture{};
    char font_path[MB(1)]{};
    TTF_Font* fonts[FONTSIZE_COUNT]{};
    int font_sizes[FONTSIZE_COUNT]{12, 16, 24, 32, 36};

    // Render frame data
    Camera2d game_camera{};
    Camera2d ui_camera{};
    SDL_GPUTexture* depth_texture{};
    ivec2 depth_texture_size{};
    Array<SpriteVertex, MAX_SPRITES> sprite_vertices{};
    TextGeometryData text_geometry{};
    Array<QueuedText, 100> queued_texts{};

    bool init();
    bool init_text(const char* fontfile_path);
    void cleanup();

    void render();
    void draw_sprite(SpriteId sprite_id, vec2 pos);
    void draw_sprite(SpriteId sprite_id, ivec2 pos);
    void draw_sprite(SpriteId sprite_id, vec2 pos, vec2 size);
    void draw_sprite(SpriteId sprite_id, ivec2 pos, vec2 size);
    void draw_text(const char* text, vec2 position, vec4 color, FontSize font_size);

  private:
    void process_queued_text();
    void upload_sprite_data();
    void upload_text_data();
    void render_sprite_vertices(
        SDL_GPURenderPass* render_pass,
        SDL_GPUCommandBuffer* cmdbuf,
        mat4x4* camera_matrix
    );
    void render_text_geometry(
        SDL_GPURenderPass* render_pass,
        SDL_GPUCommandBuffer* cmdbuf,
        mat4x4* matrices
    );
    TTF_Font* get_font(FontSize size);
};

static Renderer* renderer{};

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
