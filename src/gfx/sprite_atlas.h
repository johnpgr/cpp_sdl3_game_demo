#pragma once

#include "SDL3/SDL_gpu.h"
#include "core/arena.h"
#include "core/array.h"
#include "gfx/sprite.h"
#include "core/math3d.h"

struct SpriteAtlasEntry {
    ivec2 atlas_offset; // Pixel coordinates in atlas
    ivec2 size;         // Sprite size in pixels
    vec2 uv_min;        // Normalized UV coordinates (0-1)
    vec2 uv_max;        // Normalized UV coordinates (0-1)
    const char* name;   // Optional sprite name for debugging
};

struct SpriteAtlas {
    SDL_GPUTexture* texture{};
    SDL_GPUSampler* sampler{};
    ivec2 atlas_size{}; // Total atlas dimensions in pixels
    Array<SpriteAtlasEntry, 256> sprites{};

    void destroy();

    SpriteId register_sprite(
        ivec2 atlas_offset,
        ivec2 size,
        const char* name
    );

    void register_sprite_at_id(
        SpriteId id,
        ivec2 atlas_offset,
        ivec2 size,
        const char* name
    );

    void compute_uv_coords(
        SpriteId sprite_id,
        vec2* uv_min,
        vec2* uv_max
    ) const;

    SpriteAtlasEntry get_sprite_entry(SpriteId sprite_id) const;

    bool is_valid_sprite_id(SpriteId sprite_id) const;
};

static SpriteAtlas* sprite_atlas{};

bool init_sprite_atlas(Arena* arena, const char* atlas_filename);
void register_sprites();
