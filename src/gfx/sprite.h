#pragma once

#include "core/math3d.h"

enum SpriteId {
    SPRITE_WHITE,
    SPRITE_DICE,

    SPRITE_COUNT,
};

struct Sprite {
    ivec2 atlas_offset{};
    ivec2 size{};
};
