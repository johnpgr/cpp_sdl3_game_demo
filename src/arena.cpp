#include "arena.h"
#include "assert.h"

#include <SDL3/SDL.h>
#include <string.h> // For std::memset
#include <new>

// Helper for aligning memory
inline void* align_ptr(void* ptr, usize alignment) {
    usize current_addr = (usize)(ptr);
    usize aligned_addr = (current_addr + alignment - 1) & ~(alignment - 1);
    return (void*)(aligned_addr);
}

Arena::Arena(usize initial_block_capacity, bool can_grow)
    : can_grow(can_grow), current_block(nullptr), total_used_size(0),
      initial_block_capacity(initial_block_capacity) {
    DEBUG_ASSERT(
        initial_block_capacity != 0,
        "Initial block capacity cannot be zero."
    );
    grow_arena(initial_block_capacity);
}

void Arena::destroy() {
    // Iterate through all blocks and deallocate their memory and metadata
    // Start from the last block allocated
    ArenaBlock* current = current_block;
    while (current != nullptr) {
        // Keep track to free current block
        ArenaBlock* prev_block = current;
        current = current->next; // Move to the next block in the chain
                                 // (towards head)
        SDL_free(prev_block->memory);
        SDL_free(prev_block);
    }
    current_block = nullptr;
}

void* Arena::push(usize size, usize alignment) {
    DEBUG_ASSERT(size != 0, "Can't push 0 bytes of memory");
    DEBUG_ASSERT(alignment != 0, "Can't push with 0 byte alignment");
    DEBUG_ASSERT(
        (alignment & (alignment - 1)) == 0,
        "Alignment must be a power of two."
    );

    // Calculate aligned pointer and required size including padding
    u8* aligned_ptr_in_block = (u8*)(align_ptr(
        current_block->memory + current_block->used,
        alignment
    ));
    usize padded_size =
        (aligned_ptr_in_block - (current_block->memory + current_block->used)) +
        size;

    // Check if current block has enough space
    if (current_block->used + padded_size > current_block->capacity) {
        // If the arena is not growable, we cannot allocate a new block.
        if (!can_grow) {
            DEBUG_ASSERT(
                false,
                "Arena is not growable and has run out of memory."
            );
            return nullptr;
        }

        // Current block is full, grow the arena by allocating a new block
        usize new_block_capacity = SDL_max(
            initial_block_capacity,
            size + alignment - 1
        ); // Ensure new block can fit 'size' after alignment
        grow_arena(new_block_capacity);

        // Recalculate aligned pointer for the new block
        aligned_ptr_in_block = (u8*)(align_ptr(
            current_block->memory + current_block->used,
            alignment
        ));
        padded_size = (aligned_ptr_in_block -
                       (current_block->memory + current_block->used)) +
                      size;
    }

    // Allocate from the current block
    u8* allocated_ptr = aligned_ptr_in_block;
    // Update used bytes
    current_block->used = (allocated_ptr - current_block->memory) + size;
    // Accumulate total logical size across blocks
    total_used_size += padded_size;
    return allocated_ptr;
}

void* Arena::push_zero(usize size, usize alignment) {
    void* ptr = push(size, alignment);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void Arena::pop(usize new_total_size) {
    DEBUG_ASSERT(
        new_total_size < total_used_size,
        "Cannot pop to a position beyond current total used size."
    );

    if (new_total_size == total_used_size) {
        return; // No change
    }

    // Traverse blocks from newest to oldest to find the block containing
    // new_total_size
    usize remaining_size = new_total_size;
    ArenaBlock* target_block = nullptr;

    // Find the block that contains or comes after new_total_size
    // We traverse *backwards* from current_block_ to logically reset
    ArenaBlock* current = current_block;
    while (current) {
        if (remaining_size < current->used) {
            target_block = current;
            break;
        }
        remaining_size -= current->capacity; // Adjust for full blocks
        current = current->next; // Move to the previous block (towards head
                                 // of chain)
    }

    if (target_block) {
        // Reset used portion of this block
        target_block->used = remaining_size;
        // All blocks newer than target_block are now logically empty (or
        // completely unused for pop) For simplicity in this example, we
        // just set their `used` to 0. In a more complex "stack-like" pop,
        // you'd properly manage the `current_block_` pointer.
        ArenaBlock* temp = current_block;
        while (temp != target_block) {
            temp->used = 0;
            temp = temp->next;
        }
        current_block = target_block; // Update current_block_ to the block
                                      // where new_total_size ends
    } else {                          // new_total_size is 0 or less
        clear();
    }
    total_used_size = new_total_size;
}

void Arena::clear() {
    if (current_block) {
        // Reset used space for all blocks
        ArenaBlock* current = current_block;
        while (current) {
            current->used = 0;
            current = current->next;
        }
        // Point to the first block for new allocations
        current_block = get_first_block();
        total_used_size = 0;
    }
}

usize Arena::get_total_used_size() {
    return total_used_size;
}

template <typename T> T* Arena::push_array(usize count, usize alignment) {
    return (T*)(push(sizeof(T) * count, alignment));
}

template <typename T> T* Arena::push_array_zero(usize count, usize alignment) {
    return (T*)(push_zero(sizeof(T) * count, alignment));
}

template <typename T> T* Arena::push_struct(usize alignment) {
    void* memory = push(sizeof(T), alignment);
    return new (memory) T(); // Constructs T in uninitialized memory
}

template <typename T> T* Arena::push_struct_zero(usize alignment) {
    void* memory = push_zero(sizeof(T), alignment);
    return new (memory) T(); // Constructs T in zeroed memory
}

void Arena::grow_arena(usize min_capacity) {
    usize actual_capacity = SDL_max(min_capacity, initial_block_capacity);

    // Allocate metadata for the new block
    ArenaBlock* new_block = (ArenaBlock*)SDL_malloc(sizeof(ArenaBlock));
    // Allocate raw memory
    new_block->memory = (u8*)SDL_malloc(actual_capacity);
    new_block->capacity = actual_capacity;
    new_block->used = 0;
    // Link new block to the previous
    // (making it the head)
    new_block->next = current_block;
    // New block becomes the current active block
    current_block = new_block;
}

ArenaBlock* Arena::get_first_block() {
    ArenaBlock* current = current_block;
    if (!current) return nullptr;
    while (current->next) {
        current = current->next;
    }
    return current;
}
