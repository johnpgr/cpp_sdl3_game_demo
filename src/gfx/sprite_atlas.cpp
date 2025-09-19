#include "gfx/sprite_atlas.h"

#include "core/assert.h"
#include "gfx/renderer.h"
#include "core/utils.h"
#include <SDL3_image/SDL_image.h>

bool SpriteAtlas::init(const char* atlas_filename) {
    auto device = renderer->device;
    DEBUG_ASSERT(
        device != nullptr,
        "GPU device is null on init_sprite_atlas()"
    );

    SDL_Surface* atlas_surface = load_image(atlas_filename, 4);
    if (!atlas_surface) {
        SDL_Log("Failed to load sprite atlas: %s", atlas_filename);
        return false;
    }
    defer {
        SDL_DestroySurface(atlas_surface);
    };

    // Store atlas dimensions
    atlas_size = ivec2(atlas_surface->w, atlas_surface->h);
    SDL_Log(
        "Loaded sprite atlas: %dx%d",
        atlas_size.x,
        atlas_size.y
    );

    // Create GPU texture from surface
    texture = gpu_texture_from_surface(atlas_surface);
    if (!texture) {
        SDL_Log("Failed to create GPU texture from atlas surface");
        return false;
    }

    // Create sampler for pixel art (nearest neighbor filtering)
    SDL_GPUSamplerCreateInfo sampler_info{
        .min_filter = SDL_GPU_FILTER_NEAREST,
        .mag_filter = SDL_GPU_FILTER_NEAREST,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };

    sampler = SDL_CreateGPUSampler(device, &sampler_info);
    if (!sampler) {
        SDL_Log("Failed to create atlas sampler: %s", SDL_GetError());
        SDL_ReleaseGPUTexture(device, texture);
        return false;
    }

    return true;
}

void SpriteAtlas::register_sprites() {
    register_sprite_at_id(
        SPRITE_WHITE,
        ivec2(0, 0),
        ivec2(1, 1),
        "white_pixel"
    );
    register_sprite_at_id(
        SPRITE_DICE,
        ivec2(16, 0),
        ivec2(16, 16),
        "dice"
    );

    SDL_Log("Registered %zu sprites in atlas", sprite_atlas->sprites.size);
}

void SpriteAtlas::cleanup() {
    auto device = renderer->device;

    if (texture) {
        SDL_ReleaseGPUTexture(device, texture);
        texture = nullptr;
    }
    if (sampler) {
        SDL_ReleaseGPUSampler(device, sampler);
        sampler = nullptr;
    }

    sprites.clear();
}

SpriteId SpriteAtlas::register_sprite(
    ivec2 atlas_offset,
    ivec2 size,
    const char* name
) {
    // Validate coordinates
    DEBUG_ASSERT(
        atlas_offset.x >= 0 && atlas_offset.y >= 0,
        "Invalid atlas offset"
    );
    DEBUG_ASSERT(
        atlas_offset.x + size.x <= atlas_size.x,
        "Sprite extends beyond atlas width"
    );
    DEBUG_ASSERT(
        atlas_offset.y + size.y <= atlas_size.y,
        "Sprite extends beyond atlas height"
    );
    DEBUG_ASSERT(size.x > 0 && size.y > 0, "Invalid sprite size");

    SpriteAtlasEntry entry{};
    entry.atlas_offset = atlas_offset;
    entry.size = size;
    entry.name = name;

    // Compute normalized UV coordinates
    entry.uv_min = vec2(atlas_offset) / vec2(atlas_size);
    entry.uv_max = vec2(atlas_offset + size) / vec2(atlas_size);

    sprites.push(entry);
    return (SpriteId)(sprites.size - 1);
}

void SpriteAtlas::register_sprite_at_id(
    SpriteId id,
    ivec2 atlas_offset,
    ivec2 size,
    const char* name
) {
    // Ensure the array is large enough
    while (sprites.size <= (usize)id) {
        sprites.push(SpriteAtlasEntry{}); // Add empty entries
    }

    // Validate coordinates
    DEBUG_ASSERT(
        atlas_offset.x >= 0 && atlas_offset.y >= 0,
        "Invalid atlas offset"
    );
    DEBUG_ASSERT(
        atlas_offset.x + size.x <= atlas_size.x,
        "Sprite extends beyond atlas width"
    );
    DEBUG_ASSERT(
        atlas_offset.y + size.y <= atlas_size.y,
        "Sprite extends beyond atlas height"
    );
    DEBUG_ASSERT(size.x > 0 && size.y > 0, "Invalid sprite size");

    SpriteAtlasEntry& entry = sprites.items[id];
    entry.atlas_offset = atlas_offset;
    entry.size = size;
    entry.name = name;

    // Compute normalized UV coordinates
    entry.uv_min = vec2(atlas_offset) / vec2(atlas_size);
    entry.uv_max = vec2(atlas_offset + size) / vec2(atlas_size);
}

void SpriteAtlas::compute_uv_coords(
    SpriteId sprite_id,
    vec2* uv_min,
    vec2* uv_max
) const {
    DEBUG_ASSERT(is_valid_sprite_id(sprite_id), "Invalid sprite ID");

    const SpriteAtlasEntry& entry = sprites.items[sprite_id];
    *uv_min = entry.uv_min;
    *uv_max = entry.uv_max;
}

SpriteAtlasEntry SpriteAtlas::get_sprite_entry(SpriteId sprite_id) const {
    DEBUG_ASSERT(is_valid_sprite_id(sprite_id), "Invalid sprite ID");
    return sprites.items[sprite_id];
}

bool SpriteAtlas::is_valid_sprite_id(SpriteId sprite_id) const {
    return sprite_id >= 0 && (usize)sprite_id < sprites.size;
}
