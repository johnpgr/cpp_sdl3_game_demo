#include "renderer.h"
#include "assert.h"
#include "file.h"
#include "input.h"
#include "utils.h"
#include <SDL3_image/SDL_image.h>

ivec2 screen_to_world(ivec2 screen_pos) {
    if (renderer_state == nullptr) unreachable;
    if (input_state == nullptr) unreachable;

    Camera2d camera = renderer_state->game_camera;

    i32 x = (f32)screen_pos.x / (f32)input_state->screen_size.x *
            camera.dimensions.x; // [0; dimensions.x]

    // Offset using dimensions and position
    x += -camera.dimensions.x / 2.0f + camera.position.x;

    i32 y = (f32)screen_pos.y / (f32)input_state->screen_size.y *
            camera.dimensions.y; // [0; dimensions.y]

    // Offset using dimensions and position
    y += camera.dimensions.y / 2.0f + camera.position.y;

    return ivec2(x, y);
}

void draw_sprite(SpriteId sprite_id, vec2 pos) {
    if (renderer_state == nullptr) unreachable;

    Sprite sprite = Sprite::from_id(sprite_id);

    Transform transform{
        .atlas_offset = sprite.atlas_offset,
        .sprite_size = sprite.size,
        .position = pos - sprite.size.to_vec2() / 2.0f,
        .size = sprite.size.to_vec2(),
    };

    renderer_state->transforms.push(transform);
}

void draw_sprite(SpriteId sprite_id, ivec2 pos) {
    draw_sprite(sprite_id, pos.to_vec2());
}

SDL_Surface* load_image(const char* image_filename, i32 desired_channels) {
    char full_path[256];
    SDL_Surface* result;
    SDL_PixelFormat format;

    SDL_snprintf(
        full_path,
        sizeof(full_path),
        "assets/images/%s",
        image_filename
    );

    result = IMG_Load(full_path);
    if (!result) {
        SDL_Log("Failed to load image: %s", SDL_GetError());
        return nullptr;
    }

    if (desired_channels == 4) {
        format = SDL_PIXELFORMAT_ABGR8888;
    } else {
        SDL_DestroySurface(result);
        return nullptr;
    }

    if (result->format != format) {
        SDL_Surface* next = SDL_ConvertSurface(result, format);
        SDL_DestroySurface(result);
        result = next;
    }

    return result;
}

SDL_GPUTexture* gpu_texture_from_surface(SDL_Surface* surface) {
    SDL_GPUDevice* device = renderer_state->device;

    DEBUG_ASSERT(
        surface != nullptr && device != nullptr,
        "gpu_texture_from_surface called with invalid surface or GPUDevice is "
        "not bound."
    );

    SDL_GPUTransferBuffer* texture_transfer_buffer =
        SDL_CreateGPUTransferBuffer(
            device,
            &(SDL_GPUTransferBufferCreateInfo){
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size = (u32)surface->w * surface->h * 4,
            }
        );

    if (!texture_transfer_buffer) {
        return nullptr;
    }

    void* texture_transfer_ptr =
        SDL_MapGPUTransferBuffer(device, texture_transfer_buffer, false);
    SDL_memcpy(
        texture_transfer_ptr,
        surface->pixels,
        surface->w * surface->h * 4
    );
    SDL_UnmapGPUTransferBuffer(device, texture_transfer_buffer);

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(
        device,
        &(SDL_GPUTextureCreateInfo){
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
            .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
            .width = (u32)surface->w,
            .height = (u32)surface->h,
            .layer_count_or_depth = 1,
            .num_levels = 1,
        }
    );

    if (!texture) {
        SDL_ReleaseGPUTransferBuffer(device, texture_transfer_buffer);
        return nullptr;
    }

    SDL_GPUCommandBuffer* upload_cmdbuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(upload_cmdbuf);

    SDL_UploadToGPUTexture(
        copy_pass,
        &(SDL_GPUTextureTransferInfo){
            .transfer_buffer = texture_transfer_buffer,
            .offset = 0,
        },
        &(SDL_GPUTextureRegion){
            .texture = texture,
            .w = (u32)surface->w,
            .h = (u32)surface->h,
            .d = 1,
        },
        false
    );

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(upload_cmdbuf);
    SDL_ReleaseGPUTransferBuffer(device, texture_transfer_buffer);

    return texture;
}

SDL_GPUShader* load_shader(
    Arena* arena,
    SDL_GPUDevice* device,
    const char* shader_name,
    SDL_GPUShaderStage shader_stage,
    u32 num_samplers,
    u32 num_uniform_buffers,
    u32 num_storage_buffers,
    u32 num_storage_textures
) {
    SDL_GPUShaderFormat backend_formats = SDL_GetGPUShaderFormats(device);
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    char extension[8] = "";
    char entrypoint[16] = "main";

    if ((backend_formats & SDL_GPU_SHADERFORMAT_SPIRV) != 0) {
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        SDL_strlcpy(extension, ".spv", sizeof(extension));
    } else if ((backend_formats & SDL_GPU_SHADERFORMAT_DXIL) != 0) {
        format = SDL_GPU_SHADERFORMAT_DXIL;
        SDL_strlcpy(extension, ".dxil", sizeof(extension));
    } else if ((backend_formats & SDL_GPU_SHADERFORMAT_MSL) != 0) {
        format = SDL_GPU_SHADERFORMAT_MSL;
        SDL_strlcpy(extension, ".msl", sizeof(extension));
        SDL_strlcpy(entrypoint, "main0", sizeof(entrypoint));
    } else {
        SDL_Log("No supported shader formats available");
        return nullptr;
    }

    char shader_path[1024];
    i32 result = SDL_snprintf(
        shader_path,
        sizeof(shader_path),
        "assets/shaders/compiled/%s%s",
        shader_name,
        extension
    );

    if (result < 0 || result >= (i32)sizeof(shader_path)) {
        SDL_Log("Shader path too long or formatting error\n");
        return nullptr;
    }

    SDL_Log(
        "Loading shader %s from: %s\n",
        shader_stage == SDL_GPU_SHADERSTAGE_VERTEX ? "VERTEX" : "FRAGMENT",
        shader_path
    );

    usize code_size = file_get_size(shader_path);
    char* code = read_entire_file(arena, shader_path);

    return SDL_CreateGPUShader(
        device,
        &(SDL_GPUShaderCreateInfo){
            .code_size = code_size,
            .code = (u8*)code,
            .entrypoint = entrypoint,
            .format = format,
            .stage = shader_stage,
            .num_samplers = num_samplers,
            .num_storage_textures = num_storage_textures,
            .num_storage_buffers = num_storage_buffers,
            .num_uniform_buffers = num_uniform_buffers,
        }
    );
}
