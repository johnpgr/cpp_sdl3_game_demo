#include "gfx/renderer.h"
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_properties.h"
#include "core/assert.h"
#include "core/file.h"
#include "core/utils.h"
#include "game/consts.h"
#include "game/input.h"
#include "gfx/sprite_atlas.h"
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

void TextGeometryData::queue_text_sequence(
    TTF_GPUAtlasDrawSequence* sequence,
    vec4 color,
    vec2 offset
) {
    DEBUG_ASSERT(
        vertices.size + sequence->num_vertices <= MAX_TEXT_VERTICES,
        "Text vertex buffer overflow"
    );
    DEBUG_ASSERT(
        vertices.size + sequence->num_indices <= MAX_TEXT_INDICES,
        "Text index buffer overflow"
    );

    i32 vertex_offset = vertices.size;

    for (i32 i = 0; i < sequence->num_vertices; i++) {
        TextVertex vertex{};

        vertex.pos.x = sequence->xy[i].x + offset.x;
        vertex.pos.y = sequence->xy[i].y + offset.y;
        vertex.pos.z = 0.0f;
        vertex.color = color;
        vertex.uv = vec2(sequence->uv[i].x, sequence->uv[i].y);

        vertices.push(vertex);
    }

    for (i32 i = 0; i < sequence->num_indices; i++) {
        indices.push(sequence->indices[i] + vertex_offset);
    }
}

void TextGeometryData::reset() {
    vertices.clear();
    indices.clear();
}

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
 * @note Sets the global renderer pointer on success
 * @note Logs specific error messages for each failure point
 */
bool Renderer::init() {
    window = SDL_CreateWindow(
        "FPS: ",
        INITIAL_WINDOW_WIDTH,
        INITIAL_WINDOW_HEIGHT,
        SDL_WINDOW_HIDDEN
    );
    if (!window) {
        SDL_Log("Failed to create a window");
        return false;
    }

#ifdef _WIN32
    SDL_SetHint(SDL_HINT_GPU_DRIVER, "direct3d12");
#endif

    // clang-format off
    device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_DXIL |
        SDL_GPU_SHADERFORMAT_SPIRV |
        SDL_GPU_SHADERFORMAT_MSL,
        DEBUG_BOOL,
        nullptr
    );
    // clang-format on

    if (!device) {
        SDL_Log("Failed to create a GPU device");
        return false;
    }

    const char* device_driver = SDL_GetGPUDeviceDriver(device);
    SDL_Log("Created GPU Device with driver %s\n", device_driver);

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("Failed to claim window for GPU device %s\n", SDL_GetError());
        return false;
    }

    if (!SDL_SetGPUSwapchainParameters(
            device,
            window,
            SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
            SDL_GPU_PRESENTMODE_IMMEDIATE
        )) {
        SDL_Log("Failed to set GPU swapchain parameters");
    }

    SDL_GPUTransferBufferCreateInfo transfer_info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = (u32)(sizeof(SpriteVertex) * sprite_vertices.capacity),
    };
    SDL_GPUTransferBuffer* transfer_buffer =
        SDL_CreateGPUTransferBuffer(device, &transfer_info);
    sprite_transfer_buffer = transfer_buffer;

    SDL_GPUBufferCreateInfo vertex_buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = sizeof(QUAD_VERTICES),
    };
    sprite_quad_vertex_buffer =
        SDL_CreateGPUBuffer(device, &vertex_buffer_info);
    if (!sprite_quad_vertex_buffer) {
        SDL_Log("Failed to create quad_vertex_buffer");
        return false;
    }

    SDL_GPUBufferCreateInfo index_buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = sizeof(QUAD_INDICES),
    };
    sprite_quad_index_buffer =
        SDL_CreateGPUBuffer(device, &index_buffer_info);
    if (!sprite_quad_index_buffer) {
        SDL_Log("Failed to create quad_index_buffer");
        return false;
    }

    SDL_GPUCommandBuffer* upload_cmd =
        SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(upload_cmd);

    // Upload vertices
    SDL_GPUTransferBuffer* vertex_transfer = SDL_CreateGPUTransferBuffer(
        device,
        &(SDL_GPUTransferBufferCreateInfo){
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = sizeof(QUAD_VERTICES),
        }
    );

    void* vertex_data =
        SDL_MapGPUTransferBuffer(device, vertex_transfer, false);
    SDL_memcpy(vertex_data, QUAD_VERTICES, sizeof(QUAD_VERTICES));
    SDL_UnmapGPUTransferBuffer(device, vertex_transfer);

    SDL_UploadToGPUBuffer(
        copy_pass,
        &(SDL_GPUTransferBufferLocation){
            .transfer_buffer = vertex_transfer,
            .offset = 0,
        },
        &(SDL_GPUBufferRegion){
            .buffer = sprite_quad_vertex_buffer,
            .offset = 0,
            .size = sizeof(QUAD_VERTICES),
        },
        false
    );

    // Upload indices
    SDL_GPUTransferBuffer* index_transfer = SDL_CreateGPUTransferBuffer(
        device,
        &(SDL_GPUTransferBufferCreateInfo){
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = sizeof(QUAD_INDICES),
        }
    );

    void* index_data =
        SDL_MapGPUTransferBuffer(device, index_transfer, false);
    SDL_memcpy(index_data, QUAD_INDICES, sizeof(QUAD_INDICES));
    SDL_UnmapGPUTransferBuffer(device, index_transfer);

    SDL_UploadToGPUBuffer(
        copy_pass,
        &(SDL_GPUTransferBufferLocation){
            .transfer_buffer = index_transfer,
            .offset = 0,
        },
        &(SDL_GPUBufferRegion){
            .buffer = sprite_quad_index_buffer,
            .offset = 0,
            .size = sizeof(QUAD_INDICES),
        },
        false
    );

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(upload_cmd);

    SDL_ReleaseGPUTransferBuffer(device, vertex_transfer);
    SDL_ReleaseGPUTransferBuffer(device, index_transfer);

    SDL_GPUShader* vertex_shader = load_shader(
        "quad.vert",
        device,
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
        device,
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
        .format = SDL_GetGPUSwapchainTextureFormat(device, window),
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

    sprite_pipeline =
        SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
    if (!sprite_pipeline) {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return false;
    }

    SDL_ReleaseGPUShader(device, vertex_shader);
    SDL_ReleaseGPUShader(device, frag_shader);

    SDL_GPUBufferCreateInfo transform_buffer_info{};
    transform_buffer_info.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
    transform_buffer_info.size =
        sizeof(SpriteVertex) * sprite_vertices.capacity;

    sprite_vertex_buffer =
        SDL_CreateGPUBuffer(device, &transform_buffer_info);

    if (!sprite_vertex_buffer) {
        SDL_Log("Failed to create transform buffer: %s", SDL_GetError());
        return false;
    }

    return true;
}

bool Renderer::init_text(const char* fontfile_path) {
    SDL_GPUShader* vertex_shader = load_shader(
        "font.vert",
        device,
        SDL_GPU_SHADERSTAGE_VERTEX,
        {
            .num_samplers = 0,
            .num_uniform_buffers = 1,
            .num_storage_buffers = 0,
            .num_storage_textures = 0,
        }
    );

    SDL_GPUShader* frag_shader = load_shader(
        "font.frag",
        device,
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        {
            .num_samplers = 1,
            .num_uniform_buffers = 0,
            .num_storage_buffers = 0,
            .num_storage_textures = 0,
        }
    );

    if (!vertex_shader || !frag_shader) {
        SDL_Log("Failed to load font shaders");
        return false;
    }

    SDL_GPUColorTargetDescription color_target_description{
        .format = SDL_GetGPUSwapchainTextureFormat(device, window),
        .blend_state = {
            .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
            .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .color_blend_op = SDL_GPU_BLENDOP_ADD,
            .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
            .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_DST_ALPHA,
            .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
            .color_write_mask = 0xF,
            .enable_blend = true,
        },
    };
    SDL_GPUVertexBufferDescription vertex_buffer_descriptions{
        .slot = 0,
        .pitch = sizeof(TextVertex),
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0,
    };

    SDL_GPUVertexAttribute vertex_attributes[3]{
        {
            .location = 0,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset = 0,
        },
        {
            .location = 1,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
            .offset = sizeof(f32) * 3,
        },
        {
            .location = 2,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = sizeof(f32) * 7,
        },
    };

    // clang-format off
    SDL_GPUGraphicsPipelineCreateInfo pipeline_info{
        .vertex_shader = vertex_shader,
        .fragment_shader = frag_shader,
        .vertex_input_state = {
            .vertex_buffer_descriptions = &vertex_buffer_descriptions,
            .num_vertex_buffers = 1,
            .vertex_attributes = vertex_attributes,
            .num_vertex_attributes = 3,
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info = {
            .color_target_descriptions = &color_target_description,
            .num_color_targets = 1,
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_INVALID,
            .has_depth_stencil_target = false,
        },
    };
    // clang-format on

    SDL_GPUGraphicsPipeline* pipeline =
        SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
    if (!pipeline) {
        SDL_Log("Fail to create text pipeline");
        return false;
    }
    renderer->text_pipeline = pipeline;

    SDL_ReleaseGPUShader(device, vertex_shader);
    SDL_ReleaseGPUShader(device, frag_shader);

    SDL_GPUBufferCreateInfo vertex_buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = sizeof(TextVertex) * MAX_TEXT_VERTICES,
    };
    SDL_GPUBuffer* vertex_buffer =
        SDL_CreateGPUBuffer(device, &vertex_buffer_info);
    if (!vertex_buffer) {
        SDL_Log("Fail to create text vertex buffer");
        return false;
    }
    renderer->text_vertex_buffer = vertex_buffer;

    SDL_GPUBufferCreateInfo index_buffer_info{
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = sizeof(i32) * MAX_TEXT_INDICES,
    };
    SDL_GPUBuffer* index_buffer =
        SDL_CreateGPUBuffer(device, &index_buffer_info);
    if (!index_buffer) {
        SDL_Log("Fail to create text_index_buffer");
        return false;
    }
    renderer->text_index_buffer = index_buffer;

    SDL_GPUTransferBufferCreateInfo transfer_buffer_info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = (sizeof(TextVertex) * MAX_TEXT_VERTICES) +
                (sizeof(i32) * MAX_TEXT_INDICES),
    };
    SDL_GPUTransferBuffer* transfer_buffer =
        SDL_CreateGPUTransferBuffer(device, &transfer_buffer_info);
    if (!transfer_buffer) {
        SDL_Log("Fail to create text transfer buffer");
        return false;
    }
    renderer->text_transfer_buffer = transfer_buffer;

    SDL_GPUSamplerCreateInfo sampler_info{
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };
    SDL_GPUSampler* sampler = SDL_CreateGPUSampler(device, &sampler_info);
    if (!sampler) {
        SDL_Log("Fail to create text gpu sampler");
        return false;
    }
    renderer->text_sampler = sampler;

    TTF_Font* font = TTF_OpenFont(fontfile_path, 50);
    if (!font) {
        SDL_Log("Failed to open font: %s", SDL_GetError());
        return false;
    }
    renderer->font = font;

    auto engine = TTF_CreateGPUTextEngine(device);
    if (!engine) {
        SDL_Log("Failed to create GPU text engine: %s", SDL_GetError());
        return false;
    };
    renderer->text_engine = engine;

    return true;
}

void Renderer::destroy() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    if (text_engine) {
        TTF_DestroyGPUTextEngine(text_engine);
        text_engine = nullptr;
    }
    if (sprite_pipeline) {
        SDL_ReleaseGPUGraphicsPipeline(device, sprite_pipeline);
        sprite_pipeline = nullptr;
    }

    if (sprite_transfer_buffer) {
        SDL_ReleaseGPUTransferBuffer(device, sprite_transfer_buffer);
        sprite_transfer_buffer = nullptr;
    }

    if (sprite_vertex_buffer) {
        SDL_ReleaseGPUBuffer(device, sprite_vertex_buffer);
        sprite_vertex_buffer = nullptr;
    }

    if (sprite_quad_vertex_buffer) {
        SDL_ReleaseGPUBuffer(device, sprite_quad_vertex_buffer);
        sprite_quad_vertex_buffer = nullptr;
    }

    if (sprite_quad_index_buffer) {
        SDL_ReleaseGPUBuffer(device, sprite_quad_index_buffer);
        sprite_quad_index_buffer = nullptr;
    }

    if (text_pipeline) {
        SDL_ReleaseGPUGraphicsPipeline(device, text_pipeline);
        text_pipeline = nullptr;
    }

    if (text_vertex_buffer) {
        SDL_ReleaseGPUBuffer(device, text_vertex_buffer);
        text_vertex_buffer = nullptr;
    }

    if (text_index_buffer) {
        SDL_ReleaseGPUBuffer(device, text_index_buffer);
        text_index_buffer = nullptr;
    }

    if (text_sampler) {
        SDL_ReleaseGPUSampler(device, text_sampler);
        text_sampler = nullptr;
    }

    if (text_transfer_buffer) {
        SDL_ReleaseGPUTransferBuffer(device, text_transfer_buffer);
        text_transfer_buffer = nullptr;
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
 * @note Requires renderer and input to be initialized
 * @note Performs early exit if no transforms are queued
 * @note Creates temporary depth texture for each frame
 */
void Renderer::render() {
    // Calculate the view bounds based on the camera's position and dimensions
    float view_width = game_camera.dimensions.x / game_camera.zoom;
    float view_height = game_camera.dimensions.y / game_camera.zoom;

    float min_x = game_camera.position.x - view_width / 2.0f;
    float max_x = game_camera.position.x + view_width / 2.0f;
    float min_y = game_camera.position.y - view_height / 2.0f;
    float max_y = game_camera.position.y + view_height / 2.0f;

    mat4x4 camera_matrix =
        mat4x4::orthographic_projection(min_x, max_x, min_y, max_y);

    mat4x4 text_matrices[2]{
        mat4x4::orthographic_projection(
            0,
            input->screen_size.x,
            input->screen_size.y,
            0
        ),
        mat4x4(),
    };

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

    process_queued_text();

    if (!sprite_vertices.is_empty()) {
        upload_sprite_data();
    }

    if (text_geometry.vertices.size > 0) {
        upload_text_data();
    }

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetFloatProperty(
        props,
        SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_DEPTH_FLOAT,
        1.0f
    );

    SDL_GPUTextureCreateInfo depth_texture_info{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = (u32)input->screen_size.x,
        .height = (u32)input->screen_size.y,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .props = props,
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
            .clear_depth = 1.0f,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_DONT_CARE,
        }
    );

    if (!sprite_vertices.is_empty()) {
        render_sprite_vertices(render_pass, cmdbuf, &camera_matrix);
    }

    if (text_geometry.vertices.size > 0) {
        render_text_geometry(render_pass, cmdbuf, text_matrices);
    }

    SDL_EndGPURenderPass(render_pass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);
    SDL_ReleaseGPUTexture(device, depth_texture);

    // Clear per-frame data
    sprite_vertices.clear();
    queued_texts.clear();
    text_geometry.reset();
}

void Renderer::upload_sprite_data() {
    SDL_GPUCommandBuffer* upload_cmd = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(upload_cmd);

    void* transfer_data =
        SDL_MapGPUTransferBuffer(device, sprite_transfer_buffer, false);
    if (!transfer_data) {
        SDL_Log("Failed to map text transfer buffer");
        return;
    }

    SDL_memcpy(
        transfer_data,
        sprite_vertices.items,
        sizeof(SpriteVertex) * sprite_vertices.size
    );
    SDL_UnmapGPUTransferBuffer(device, sprite_transfer_buffer);

    SDL_UploadToGPUBuffer(
        copy_pass,
        &(SDL_GPUTransferBufferLocation){
            .transfer_buffer = sprite_transfer_buffer,
        },
        &(SDL_GPUBufferRegion){
            .buffer = sprite_vertex_buffer,
            .size = (u32)(sizeof(SpriteVertex) * sprite_vertices.size),
        },
        false
    );

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(upload_cmd);
}

void Renderer::upload_text_data() {
    SDL_GPUCommandBuffer* upload_cmd = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(upload_cmd);

    void* transfer_data =
        SDL_MapGPUTransferBuffer(device, text_transfer_buffer, false);
    if (!transfer_data) {
        SDL_Log("Failed to map text transfer buffer");
        return;
    }

    usize vertex_bytes = sizeof(TextVertex) * text_geometry.vertices.size;
    usize index_bytes = sizeof(i32) * text_geometry.indices.size;
    usize index_buffer_offset = sizeof(TextVertex) * MAX_TEXT_VERTICES;

    // Copy vertex data
    u8* dst_vertices = (u8*)transfer_data;
    SDL_memcpy(dst_vertices, text_geometry.vertices.items, vertex_bytes);

    // Copy index data
    u8* dst_indices = (u8*)transfer_data + index_buffer_offset;
    SDL_memcpy(dst_indices, text_geometry.indices.items, index_bytes);

    SDL_UnmapGPUTransferBuffer(device, text_transfer_buffer);

    // Upload vertices
    SDL_UploadToGPUBuffer(
        copy_pass,
        &(SDL_GPUTransferBufferLocation){
            .transfer_buffer = text_transfer_buffer,
            .offset = 0,
        },
        &(SDL_GPUBufferRegion){
            .buffer = text_vertex_buffer,
            .offset = 0,
            .size = (u32)vertex_bytes,
        },
        false
    );

    // Upload indices
    SDL_UploadToGPUBuffer(
        copy_pass,
        &(SDL_GPUTransferBufferLocation){
            .transfer_buffer = text_transfer_buffer,
            .offset = (u32)index_buffer_offset,
        },
        &(SDL_GPUBufferRegion){
            .buffer = text_index_buffer,
            .offset = 0,
            .size = (u32)index_bytes,
        },
        false
    );

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(upload_cmd);
}

void Renderer::process_queued_text() {
    // Process queued text into geometry data
    for (usize i = 0; i < queued_texts.size; i++) {
        QueuedText* queued = &queued_texts[i];
        if (!queued) {
            continue;
        }

        TTF_Text* ttf_text = TTF_CreateText(text_engine, font, queued->text, 0);
        if (ttf_text) {
            TTF_GPUAtlasDrawSequence* sequence =
                TTF_GetGPUTextDrawData(ttf_text);
            if (sequence) {
                if (!text_atlas_texture && sequence->atlas_texture) {
                    text_atlas_texture = sequence->atlas_texture;
                }
                text_geometry.queue_text_sequence(
                    sequence,
                    queued->color,
                    queued->position
                );
            }
            TTF_DestroyText(ttf_text);
        }
    }
}

void Renderer::render_sprite_vertices(
    SDL_GPURenderPass* render_pass,
    SDL_GPUCommandBuffer* cmdbuf,
    mat4x4* camera_matrix
) {
    SDL_BindGPUGraphicsPipeline(render_pass, sprite_pipeline);
    SDL_PushGPUVertexUniformData(cmdbuf, 0, camera_matrix, sizeof(mat4x4));
    SDL_BindGPUVertexStorageBuffers(render_pass, 0, &sprite_vertex_buffer, 1);
    SDL_BindGPUVertexBuffers(
        render_pass,
        0,
        &(SDL_GPUBufferBinding){.buffer = sprite_quad_vertex_buffer},
        1
    );
    SDL_BindGPUIndexBuffer(
        render_pass,
        &(SDL_GPUBufferBinding){.buffer = sprite_quad_index_buffer},
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
    SDL_DrawGPUIndexedPrimitives(
        render_pass,
        6,
        (u32)sprite_vertices.size,
        0,
        0,
        0
    );
}

void Renderer::render_text_geometry(
    SDL_GPURenderPass* render_pass,
    SDL_GPUCommandBuffer* cmdbuf,
    mat4x4* matrices
) {
    SDL_BindGPUGraphicsPipeline(render_pass, text_pipeline);
    SDL_BindGPUVertexBuffers(
        render_pass,
        0,
        &(SDL_GPUBufferBinding){
            .buffer = text_vertex_buffer,
            .offset = 0,
        },
        1
    );
    SDL_BindGPUIndexBuffer(
        render_pass,
        &(SDL_GPUBufferBinding){
            .buffer = text_index_buffer,
            .offset = 0,
        },
        SDL_GPU_INDEXELEMENTSIZE_32BIT
    );
    SDL_PushGPUVertexUniformData(cmdbuf, 0, matrices, sizeof(mat4x4) * 2);

    SDL_BindGPUFragmentSamplers(
        render_pass,
        0,
        &(SDL_GPUTextureSamplerBinding){
            .texture = text_atlas_texture,
            .sampler = text_sampler,
        },
        1
    );
    SDL_DrawGPUIndexedPrimitives(
        render_pass,
        text_geometry.indices.size,
        1, 0, 0, 0
    );
}

/**
 * @brief Queues a sprite for rendering at the specified world position with
 * original size.
 *
 * Retrieves sprite data from the global sprite atlas and creates a transform
 * for instanced rendering. The sprite is positioned so that the specified
 * coordinates represent the center of the sprite.
 *
 * @param sprite_id ID of the sprite in the sprite atlas
 * @param pos World position where the sprite center should be placed
 *
 * @note Requires sprite_atlas and renderer_state to be initialized
 * @note Sprite position is offset by half the sprite size to convert from
 * center to top-left
 * @note Transform is added to the render queue for the current frame
 */
void Renderer::draw_sprite(SpriteId sprite_id, vec2 pos) {
    DEBUG_ASSERT(
        sprite_atlas != nullptr,
        "sprite_atlas is null at draw_sprite()"
    );
    DEBUG_ASSERT(
        renderer != nullptr,
        "renderer_state is null at draw_sprite()"
    );

    SpriteAtlasEntry sprite = sprite_atlas->get_sprite_entry(sprite_id);

    SpriteVertex sprite_vertex{
        .pos = pos - vec2(sprite.size) / 2.0f,
        .size = vec2(sprite.size),
        .uv_min = sprite.uv_min,
        .uv_max = sprite.uv_max,
    };

    renderer->sprite_vertices.push(sprite_vertex);
}

/**
 * @brief Integer position overload for draw_sprite.
 *
 * Convenience overload that converts integer coordinates to floating point
 * and calls the main draw_sprite function.
 *
 * @param sprite_id ID of the sprite in the sprite atlas
 * @param pos World position as integer coordinates (sprite center)
 */
void Renderer::draw_sprite(SpriteId sprite_id, ivec2 pos) {
    draw_sprite(sprite_id, vec2(pos));
}

/**
 * @brief Queues a sprite for rendering with custom size scaling.
 *
 * Similar to the basic draw_sprite but allows overriding the sprite size for
 * scaling effects. The original UV coordinates from the sprite atlas are
 * preserved, so the entire sprite texture is mapped to the custom size.
 *
 * @param sprite_id ID of the sprite in the sprite atlas
 * @param pos World position where the sprite center should be placed
 * @param size Custom size for rendering (overrides atlas size)
 *
 * @note Maintains original UV coordinates for proper texture sampling
 * @note Useful for scaling sprites without creating new atlas entries
 */
void Renderer::draw_sprite(SpriteId sprite_id, vec2 pos, vec2 size) {
    DEBUG_ASSERT(
        sprite_atlas != nullptr,
        "sprite_atlas is null at draw_sprite()"
    );
    DEBUG_ASSERT(
        renderer != nullptr,
        "renderer_state is null at draw_sprite()"
    );

    SpriteAtlasEntry sprite = sprite_atlas->get_sprite_entry(sprite_id);

    SpriteVertex sprite_vertex{
        .pos = pos - size / 2.0f,
        .size = size,
        .uv_min = sprite.uv_min,
        .uv_max = sprite.uv_max,
    };

    renderer->sprite_vertices.push(sprite_vertex);
}

/**
 * @brief Integer position overload for draw_sprite with custom sizing.
 *
 * Convenience overload that converts integer coordinates to floating point
 * and calls the main draw_sprite function with custom size.
 *
 * @param sprite_id ID of the sprite in the sprite atlas
 * @param pos World position as integer coordinates (sprite center)
 * @param size Custom size for rendering
 */
void Renderer::draw_sprite(SpriteId sprite_id, ivec2 pos, vec2 size) {
    draw_sprite(sprite_id, vec2(pos), size);
}

void Renderer::draw_text(const char* text, vec2 position, vec4 color) {
    if (queued_texts.is_full()) {
        SDL_Log("Text queue is full, skipping text: %s", text);
        return;
    }
    QueuedText queued_text{};
    SDL_strlcpy(queued_text.text, text, sizeof(queued_text.text));
    queued_text.position = position;
    queued_text.color = color;

    queued_texts.push(queued_text);
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
 * @note Requires both renderer and input to be initialized
 * @note Y-axis transformation handles screen Y increasing downward vs world Y
 * direction
 */
ivec2 screen_to_world(ivec2 screen_pos) {
    if (renderer == nullptr) unreachable;
    if (input == nullptr) unreachable;

    Camera2d camera = renderer->game_camera;

    i32 x = (f32)screen_pos.x / (f32)input->screen_size.x *
            camera.dimensions.x; // [0; dimensions.x]

    // Offset using dimensions and position
    x += -camera.dimensions.x / 2.0f + camera.position.x;

    i32 y = (f32)screen_pos.y / (f32)input->screen_size.y *
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
 * @note Requires renderer->device to be initialized
 * @note Creates and manages temporary transfer buffer for upload
 * @note Automatically submits upload commands and cleans up transfer resources
 * @note Typically used after load_image() to create GPU-resident textures
 * @note Caller is responsible for releasing the returned texture with
 * SDL_ReleaseGPUTexture
 */
SDL_GPUTexture* gpu_texture_from_surface(SDL_Surface* surface) {
    SDL_GPUDevice* device = renderer->device;

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
 * @note Requires renderer->device to be initialized
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
