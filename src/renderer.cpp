#include "renderer.h"
#include "input.h"
#include "utils.h"

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
