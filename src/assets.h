#pragma once

#include "math3d.h"
#include "types.h"

enum SpriteId : u8 {
    SPRITE_WHITE,
    SPRITE_DICE,

    SPRITE_COUNT,
};

struct Sprite {
    ivec2 atlas_offset{};
    ivec2 size{};

    static Sprite from_id(SpriteId id);
};
