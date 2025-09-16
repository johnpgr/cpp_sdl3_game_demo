#pragma once

#include "types.h"

static void* platform_alloc(usize size);
static void platform_free(void* ptr, usize size);
