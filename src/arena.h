#pragma once

#include "types.h"

// Structure for a memory block in the arena chain
struct ArenaBlock {
    u8* memory;       // Pointer to the allocated raw memory for this block
    usize capacity;   // Total capacity of this block
    usize used;       // Currently used bytes in this block
    ArenaBlock* next; // Pointer to the next block in the chain
};

struct Arena {
    // Constructor: Initializes the arena with an initial block capacity.
    // New blocks will be at least this size, or larger if a single allocation
    // request exceeds it.
    Arena(usize initial_block_capacity = 4096);

    // Destructor: Frees all memory blocks owned by the arena.
    void destroy();

    // Push: Allocates 'size' bytes from the arena, with specified alignment.
    // Returns a pointer to the allocated memory.
    void* push(usize size, usize alignment = 8);

    // PushZero: Allocates 'size' bytes and initializes them to zero.
    void* push_zero(usize size, usize alignment = 8);

    // Pop: Resets the total allocated size to `new_total_size`.
    // Effectively deallocates all memory pushed after `new_total_size`.
    // This is a "logical" pop; memory within blocks is only marked as unused.
    void pop(usize new_total_size);

    // ArenaClear: Resets the arena, making all its memory available for reuse.
    // Does not free the underlying memory blocks to the OS.
    void clear();

    // Get current total allocated size across all blocks.
    usize get_total_used_size();

    // Helper macro for C-style convenience
    template <typename T>
    T* push_array(usize count, usize alignment = alignof(T));

    template <typename T>
    T* push_array_zero(usize count, usize alignment = alignof(T));

    template <typename T> T* push_struct(usize alignment = alignof(T));

    template <typename T> T* push_struct_zero(usize alignment = alignof(T));

  private:
    // Pointer to the current active block for allocations*
    ArenaBlock* current_block;
    // Accumulative logical size of all allocations
    usize total_used_size;
    // Default capacity for new blocks
    usize initial_block_capacity;

    // Internal function to allocate a new memory block and add it to the chain.
    void grow_arena(usize min_capacity);

    // Helper to get the very first block in the chain (oldest block)
    ArenaBlock* get_first_block();
};
