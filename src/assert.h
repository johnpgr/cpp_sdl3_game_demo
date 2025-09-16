#pragma once

#include <SDL3/SDL_log.h>

// Function declarations
bool is_debugger_attached();
void print_stack_trace();

// Macro definitions
#define DEBUG_BREAK()                                                          \
    do {                                                                       \
        if (is_debugger_attached()) {                                          \
            __builtin_trap();                                                  \
        }                                                                      \
    } while (0)

#define DEBUG_ASSERT(condition, message)                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            SDL_Log("Assertion failed: %s", #condition);                           \
            SDL_Log("Message: %s", message);                                       \
            SDL_Log("File: %s", __FILE__);                                         \
            SDL_Log("Line: %d", __LINE__);                                         \
                                                                               \
            if (is_debugger_attached()) {                                      \
                DEBUG_BREAK();                                                 \
            } else {                                                           \
                print_stack_trace();                                           \
                abort();                                                  \
            }                                                                  \
        }                                                                      \
    } while (0)
