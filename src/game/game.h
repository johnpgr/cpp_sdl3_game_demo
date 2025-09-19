#pragma once

#include "core/utils.h"
#include "game/game_state.h"
#include "gfx/renderer.h"
#include "gfx/sprite_atlas.h"

EXPORT_FN void game_update(GameState* gs, Input* is, SpriteAtlas* sa, Renderer* r);
