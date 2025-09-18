#include "renderer.h"
#include "SDL3/SDL_gpu.h"
#include "assert.h"
#include "consts.h"
#include "file.h"
#include "input.h"
#include "sprite_atlas.h"
#include "utils.h"
#include <SDL3_image/SDL_image.h>

// clang-format off
static const f32 QUAD_VERTICES[] = {
    // Position    UV
    0.0f, 0.0f, 0.0f, 0.0f, // Top-left
    1.0f, 0.0f, 1.0f, 0.0f, // Top-right
    0.0f, 1.0f, 0.0f, 1.0f, // Bottom-left
    1.0f, 1.0f, 1.0f, 1.0f // Bottom-right
};

static const u16 QUAD_INDICES[] = {
    0, 1, 2, // First triangle
    2, 1, 3 // Second triangle
};
// clang-format on

/**
 * @brief Initializes the renderer state and sets up the GPU pipeline for
 * rendering sprites.
 *
 * This function performs complete renderer initialization including:
 * - Creating SDL window with specified dimensions
 * - Initializing GPU device with multi-format shader support (DXIL, SPIRV, MSL)
 * - Setting up vertex and index buffers for quad rendering
 * - Loading and compiling vertex and fragment shaders
 * - Configuring graphics pipeline with alpha blending and depth testing
 * - Creating transform storage buffer for instanced rendering
 *
 * @param arena Memory arena for allocating the renderer state
 * @return true on successful initialization, false on any failure
 *
 * @note Sets the global renderer_state pointer on success
 * @note Logs specific error messages for each failure point
 */
bool init_renderer_state(Arena* arena) {
    auto state = arena->push_struct<RendererState>();
    state->game_camera.dimensions = vec2(WIDTH, HEIGHT);
    state->game_camera.position = vec2(160, -90);
    state->ui_camera.dimensions = vec2(WIDTH, HEIGHT);
    state->ui_camera.position = vec2(160, -90);

    state->window = SDL_CreateWindow(
        "FPS: ",
        INITIAL_WINDOW_WIDTH,
        INITIAL_WINDOW_HEIGHT,
        SDL_WINDOW_HIDDEN
    );
    if (!state->window) {
        SDL_Log("Failed to create a window");
        return false;
    }

#ifdef _WIN32
    SDL_SetHint(SDL_HINT_GPU_DRIVER, "direct3d12");
#endif

    // clang-format off
    state->device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_DXIL |
        SDL_GPU_SHADERFORMAT_SPIRV |
        SDL_GPU_SHADERFORMAT_MSL,
        DEBUG_BOOL,
        nullptr
    );
    // clang-format on

    if (!state->device) {
        SDL_Log("Failed to create a GPU device");
        return false;
    }

    const char* device_driver = SDL_GetGPUDeviceDriver(state->device);
    SDL_Log("Created GPU Device with driver %s\n", device_driver);

    if (!SDL_ClaimWindowForGPUDevice(state->device, state->window)) {
        SDL_Log("Failed to claim window for GPU device %s\n", SDL_GetError());
        return false;
    }

    if (!SDL_SetGPUSwapchainParameters(
            state->device,
            state->window,
            SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
            SDL_GPU_PRESENTMODE_IMMEDIATE
        )) {
        SDL_Log("Failed to set GPU swapchain parameters");
    }

    SDL_GPUBufferCreateInfo vertex_buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = sizeof(QUAD_VERTICES),
    };
    state->quad_vertex_buffer =
        SDL_CreateGPUBuffer(state->device, &vertex_buffer_info);
    if (!state->quad_vertex_buffer) {
        SDL_Log("Failed to create quad_vertex_buffer");
        return false;
    }

    SDL_GPUBufferCreateInfo index_buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = sizeof(QUAD_INDICES),
    };
    state->quad_index_buffer =
        SDL_CreateGPUBuffer(state->device, &index_buffer_info);
    if (!state->quad_index_buffer) {
        SDL_Log("Failed to create quad_index_buffer");
        return false;
    }

    SDL_GPUCommandBuffer* upload_cmd =
        SDL_AcquireGPUCommandBuffer(state->device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(upload_cmd);

    // Upload vertices
    SDL_GPUTransferBuffer* vertex_transfer = SDL_CreateGPUTransferBuffer(
        state->device,
        &(SDL_GPUTransferBufferCreateInfo){
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = sizeof(QUAD_VERTICES),
        }
    );

    void* vertex_data =
        SDL_MapGPUTransferBuffer(state->device, vertex_transfer, false);
    SDL_memcpy(vertex_data, QUAD_VERTICES, sizeof(QUAD_VERTICES));
    SDL_UnmapGPUTransferBuffer(state->device, vertex_transfer);

    SDL_UploadToGPUBuffer(
        copy_pass,
        &(SDL_GPUTransferBufferLocation){
            .transfer_buffer = vertex_transfer,
            .offset = 0,
        },
        &(SDL_GPUBufferRegion){
            .buffer = state->quad_vertex_buffer,
            .offset = 0,
            .size = sizeof(QUAD_VERTICES),
        },
        false
    );

    // Upload indices
    SDL_GPUTransferBuffer* index_transfer = SDL_CreateGPUTransferBuffer(
        state->device,
        &(SDL_GPUTransferBufferCreateInfo){
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = sizeof(QUAD_INDICES),
        }
    );

    void* index_data =
        SDL_MapGPUTransferBuffer(state->device, index_transfer, false);
    SDL_memcpy(index_data, QUAD_INDICES, sizeof(QUAD_INDICES));
    SDL_UnmapGPUTransferBuffer(state->device, index_transfer);

    SDL_UploadToGPUBuffer(
        copy_pass,
        &(SDL_GPUTransferBufferLocation){
            .transfer_buffer = index_transfer,
            .offset = 0,
        },
        &(SDL_GPUBufferRegion){
            .buffer = state->quad_index_buffer,
            .offset = 0,
            .size = sizeof(QUAD_INDICES),
        },
        false
    );

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(upload_cmd);

    SDL_ReleaseGPUTransferBuffer(state->device, vertex_transfer);
    SDL_ReleaseGPUTransferBuffer(state->device, index_transfer);

    SDL_GPUShader* vertex_shader = load_shader(
        "quad.vert",
        state->device,
        SDL_GPU_SHADERSTAGE_VERTEX,
        {
            .num_samplers = 0,
            .num_uniform_buffers = 1,
            .num_storage_buffers = 1,
            .num_storage_textures = 0,
        }
    );
    if (!vertex_shader) {
        SDL_Log("Failed to load vertex shader");
        return false;
    }

    SDL_GPUShader* frag_shader = load_shader(
        "quad.frag",
        state->device,
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        {
            .num_samplers = 1,
            .num_uniform_buffers = 0,
            .num_storage_buffers = 0,
            .num_storage_textures = 0,
        }
    );
    if (!frag_shader) {
        SDL_Log("Failed to load fragment shader");
        return false;
    }

    SDL_GPUVertexAttribute vertex_attributes[2]{
        {
            .location = 0,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, // position
            .offset = 0,
        },
        {
            .location = 1,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, // uv
            .offset = sizeof(f32) * 2,
        }
    };

    SDL_GPUVertexBufferDescription vertex_buffer_desc = {
        .slot = 0,
        .pitch = sizeof(float) * 4, // (2 floats for pos, 2 for uv)
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0,
    };

    SDL_GPUVertexInputState vertex_input_state{
        .vertex_buffer_descriptions = &vertex_buffer_desc,
        .num_vertex_buffers = 1,
        .vertex_attributes = vertex_attributes,
        .num_vertex_attributes = 2,
    };

    SDL_GPURasterizerState rasterizer_state{
        .fill_mode = SDL_GPU_FILLMODE_FILL,
        .cull_mode = SDL_GPU_CULLMODE_NONE,
        .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
    };

    SDL_GPUMultisampleState multisample_state{
        .sample_count = SDL_GPU_SAMPLECOUNT_1,
    };

    SDL_GPUDepthStencilState depth_stencil_state{
        .compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
        .back_stencil_state = {},
        .front_stencil_state = {},
        .compare_mask = 0,
        .write_mask = 0,
    };

    SDL_GPUColorTargetBlendState blend_state{
        .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
        .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .color_blend_op = SDL_GPU_BLENDOP_ADD,
        .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
        .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
        .color_write_mask = 0xF,
        .enable_blend = true,
    };

    SDL_GPUColorTargetDescription color_target{
        .format =
            SDL_GetGPUSwapchainTextureFormat(state->device, state->window),
        .blend_state = blend_state,
    };

    SDL_GPUGraphicsPipelineTargetInfo target_info{
        .color_target_descriptions = &color_target,
        .num_color_targets = 1,
        .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        .has_depth_stencil_target = true,
    };

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info{
        .vertex_shader = vertex_shader,
        .fragment_shader = frag_shader,
        .vertex_input_state = vertex_input_state,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = rasterizer_state,
        .multisample_state = multisample_state,
        .depth_stencil_state = depth_stencil_state,
        .target_info = target_info,
    };

    state->pipeline =
        SDL_CreateGPUGraphicsPipeline(state->device, &pipeline_info);
    if (!state->pipeline) {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return false;
    }

    SDL_ReleaseGPUShader(state->device, vertex_shader);
    SDL_ReleaseGPUShader(state->device, frag_shader);

    SDL_GPUBufferCreateInfo transform_buffer_info{};
    transform_buffer_info.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
    transform_buffer_info.size = sizeof(Transform) * state->transforms.capacity;

    state->transform_buffer =
        SDL_CreateGPUBuffer(state->device, &transform_buffer_info);

    if (!state->transform_buffer) {
        SDL_Log("Failed to create transform buffer: %s", SDL_GetError());
        return false;
    }

    renderer_state = state;
    return true;
}

void RendererState::destroy() {
    if (pipeline) {
        SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
        pipeline = nullptr;
    }

    if (transform_buffer) {
        SDL_ReleaseGPUBuffer(device, transform_buffer);
        transform_buffer = nullptr;
    }

    if (quad_vertex_buffer) {
        SDL_ReleaseGPUBuffer(device, quad_vertex_buffer);
        quad_vertex_buffer = nullptr;
    }

    if (quad_index_buffer) {
        SDL_ReleaseGPUBuffer(device, quad_index_buffer);
        quad_index_buffer = nullptr;
    }

    if (device && window) {
        SDL_ReleaseWindowFromGPUDevice(device, window);
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    if (device) {
        SDL_DestroyGPUDevice(device);
        device = nullptr;
    }
}

/**
 * @brief Main rendering function that draws all queued sprites to the screen.
 *
 * Performs the complete rendering pipeline:
 * - Calculates orthographic projection matrix based on camera settings
 * - Acquires swapchain texture for rendering target
 * - Updates transform buffer with current frame data
 * - Sets up render pass with depth testing and alpha blending
 * - Executes instanced draw call for all sprites
 * - Submits command buffer for GPU execution
 *
 * The camera projection uses:
 * - Base view size of 200 units adjusted by zoom
 * - Aspect ratio correction based on screen dimensions
 * - Purple clear color (119, 33, 111, 255)
 *
 * @note Requires renderer_state and input_state to be initialized
 * @note Performs early exit if no transforms are queued
 * @note Creates temporary depth texture for each frame
 */
void RendererState::render() {
    if (renderer_state->transforms.is_empty()) {
        return;
    }

    // Calculate the view bounds based on the camera's position and dimensions
    float view_width = game_camera.dimensions.x / game_camera.zoom;
    float view_height = game_camera.dimensions.y / game_camera.zoom;

    float min_x = game_camera.position.x - view_width / 2.0f;
    float max_x = game_camera.position.x + view_width / 2.0f;
    float min_y = game_camera.position.y - view_height / 2.0f;
    float max_y = game_camera.position.y + view_height / 2.0f;

    mat4x4 camera_matrix =
        mat4x4::orthographic_projection(min_x, max_x, min_y, max_y);

    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(device);
    if (!cmdbuf) {
        SDL_Log("Failed to acquire command buffer %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchain_texture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            cmdbuf,
            window,
            &swapchain_texture,
            nullptr,
            nullptr
        )) {
        SDL_Log("Failed to acquire swapchain texture %s", SDL_GetError());
        return;
    }

    { // UPDATE TRANSFORM BUFFER
        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmdbuf);
        SDL_GPUTransferBufferCreateInfo transfer_info{
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = (u32)(sizeof(Transform) * transforms.size),
        };
        SDL_GPUTransferBuffer* transfer_buffer =
            SDL_CreateGPUTransferBuffer(device, &transfer_info);

        defer {
            SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
            SDL_EndGPUCopyPass(copy_pass);
        };

        void* data = SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
        SDL_memcpy(data, transforms.items, sizeof(Transform) * transforms.size);
        SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

        SDL_UploadToGPUBuffer(
            copy_pass,
            &(SDL_GPUTransferBufferLocation){
                .transfer_buffer = transfer_buffer,
                .offset = 0,
            },
            &(SDL_GPUBufferRegion){
                .buffer = transform_buffer,
                .offset = 0,
                .size = (u32)(sizeof(Transform) * transforms.size),
            },
            false
        );
    }

    SDL_GPUTextureCreateInfo depth_texture_info{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = (u32)input_state->screen_size[0],
        .height = (u32)input_state->screen_size[1],
        .layer_count_or_depth = 1,
        .num_levels = 1,
    };

    SDL_GPUTexture* depth_texture =
        SDL_CreateGPUTexture(device, &depth_texture_info);

    SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(
        cmdbuf,
        &(SDL_GPUColorTargetInfo){
            .texture = swapchain_texture,
            .clear_color =
                {119.0f / 255.0f, 33.0f / 255.0f, 111.0f / 255.0f, 1.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        },
        1,
        &(SDL_GPUDepthStencilTargetInfo){
            .texture = depth_texture,
            .clear_depth = 0.0f,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_DONT_CARE,
        }
    );

    SDL_BindGPUGraphicsPipeline(render_pass, pipeline);
    SDL_PushGPUVertexUniformData(cmdbuf, 0, &camera_matrix, sizeof(mat4x4));
    SDL_BindGPUVertexStorageBuffers(render_pass, 0, &transform_buffer, 1);

    SDL_BindGPUVertexBuffers(
        render_pass,
        0,
        &(SDL_GPUBufferBinding){
            .buffer = renderer_state->quad_vertex_buffer,
            .offset = 0,
        },
        1
    );

    SDL_BindGPUIndexBuffer(
        render_pass,
        &(SDL_GPUBufferBinding){
            .buffer = renderer_state->quad_index_buffer,
            .offset = 0,
        },
        SDL_GPU_INDEXELEMENTSIZE_16BIT
    );

    SDL_BindGPUFragmentSamplers(
        render_pass,
        0,
        &(SDL_GPUTextureSamplerBinding){
            .texture = sprite_atlas->texture,
            .sampler = sprite_atlas->sampler,
        },
        1
    );

    SDL_DrawGPUIndexedPrimitives(render_pass, 6, (u32)transforms.size, 0, 0, 0);
    SDL_EndGPURenderPass(render_pass);

    SDL_SubmitGPUCommandBuffer(cmdbuf);
    SDL_ReleaseGPUTexture(device, depth_texture);
    transforms.clear();
}

/**
 * @brief Converts screen pixel coordinates to world coordinates using the game
 * camera.
 *
 * Transforms 2D screen coordinates (where origin is top-left) to world
 * coordinates (where origin and orientation depend on camera settings).
 * Accounts for camera position, dimensions, and screen aspect ratio.
 *
 * Transformation formula:
 * - world_x = (screen_x / screen_width) * camera_width - camera_width/2 +
 * camera_pos_x
 * - world_y = (screen_y / screen_height) * camera_height + camera_height/2 +
 * camera_pos_y
 *
 * @param screen_pos Screen coordinates in pixels with origin at top-left
 * @return ivec2 World coordinates in game units
 *
 * @note Requires both renderer_state and input_state to be initialized
 * @note Y-axis transformation handles screen Y increasing downward vs world Y
 * direction
 */
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

/**
 * @brief Loads an image file from the assets directory and converts it to the
 * specified format.
 *
 * Loads an image using SDL_image from the assets/images/ directory and converts
 * it to a format suitable for GPU usage. Currently only supports 4-channel RGBA
 * format.
 *
 * @param image_filename Name of the image file in the assets/images/ directory
 * @param desired_channels Number of color channels (only 4/RGBA currently
 * supported)
 * @return SDL_Surface* Loaded and converted image surface, or nullptr on
 * failure
 *
 * @note Automatically constructs full path by prepending "assets/images/"
 * @note Converts to ABGR8888 format for GPU compatibility
 * @note Returns nullptr and logs error if loading or conversion fails
 * @note Caller is responsible for freeing the returned surface with
 * SDL_DestroySurface
 */
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

/**
 * @brief Creates a GPU texture from an SDL surface and uploads the pixel data.
 *
 * Takes an SDL surface containing image data and creates a corresponding GPU
 * texture with the data uploaded and ready for use in shaders. The texture uses
 * R8G8B8A8_UNORM format and is configured for sampler usage.
 *
 * @param surface SDL surface containing the image data to upload
 * @return SDL_GPUTexture* Ready-to-use GPU texture, or nullptr on failure
 *
 * @note Requires renderer_state->device to be initialized
 * @note Creates and manages temporary transfer buffer for upload
 * @note Automatically submits upload commands and cleans up transfer resources
 * @note Typically used after load_image() to create GPU-resident textures
 * @note Caller is responsible for releasing the returned texture with
 * SDL_ReleaseGPUTexture
 */
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

/**
 * @brief Loads and compiles a shader for the current GPU backend.
 *
 * Detects the supported shader format for the current GPU device and loads the
 * corresponding compiled shader bytecode. Supports SPIRV (Vulkan), DXIL
 * (DirectX 12), and MSL (Metal) formats with automatic format selection and
 * platform-specific entry point handling.
 *
 * Expected shader file locations:
 * - SPIRV: assets/shaders/compiled/[shader_name].spv
 * - DXIL: assets/shaders/compiled/[shader_name].dxil
 * - MSL: assets/shaders/compiled/[shader_name].msl
 *
 * @param shader_name Base name of the shader file (without extension)
 * @param shader_stage Vertex or fragment shader stage
 * @param shader_props Resource binding configuration (samplers, buffers,
 * textures)
 * @return SDL_GPUShader* Compiled shader object, or nullptr on failure
 *
 * @note Requires renderer_state->device to be initialized
 * @note Automatically selects appropriate shader format based on GPU
 * capabilities
 * @note Uses "main" entry point for SPIRV/DXIL, "main0" for MSL
 * @note Logs the selected format and shader path for debugging
 * @note Caller is responsible for releasing the shader with
 * SDL_ReleaseGPUShader
 */
SDL_GPUShader* load_shader(
    const char* shader_name,
    SDL_GPUDevice* device,
    SDL_GPUShaderStage shader_stage,
    ShaderProps shader_props
) {
    Arena arena(MAX_SHADER_FILESIZE, false);
    defer {
        arena.destroy();
    };

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
    char* code = read_entire_file(&arena, shader_path);

    return SDL_CreateGPUShader(
        device,
        &(SDL_GPUShaderCreateInfo){
            .code_size = code_size,
            .code = (u8*)code,
            .entrypoint = entrypoint,
            .format = format,
            .stage = shader_stage,
            .num_samplers = shader_props.num_samplers,
            .num_storage_textures = shader_props.num_storage_textures,
            .num_storage_buffers = shader_props.num_storage_buffers,
            .num_uniform_buffers = shader_props.num_uniform_buffers,
        }
    );
}
