#pragma once

#include "math3d.h"
#include "types.h"
#include "utils.h"

enum SpriteId : u8 {
    SPRITE_WHITE,
    SPRITE_DICE,

    SPRITE_COUNT,
};

struct Sprite {
    ivec2 atlas_offset;
    ivec2 size;

    static Sprite from_id(SpriteId id) { 
        Sprite sprite{};

        switch (id) {
            case SPRITE_WHITE: {
                sprite.atlas_offset = ivec2(0);
                sprite.size = ivec2(1);
            } break;
            case SPRITE_DICE: {
                sprite.atlas_offset = ivec2(16, 0);
                sprite.size = ivec2(16);
            } break;
            case SPRITE_COUNT: unreachable;
        }

        return sprite;
    }
};
