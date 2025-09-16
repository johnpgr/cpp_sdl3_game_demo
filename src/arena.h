#pragma once

#include "memory.h"
#include "types.h"
#include <assert.h>
#include <algorithm> // For std::max
#include <string.h> // For std::memset

// Helper for aligning memory
inline void* align_ptr(void* ptr, usize alignment) {
    usize current_addr = (usize)(ptr);
    usize aligned_addr = (current_addr + alignment - 1) & ~(alignment - 1);
    return (void*)(aligned_addr);
}

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
    Arena(usize initial_block_capacity = 4096)
        : current_block_(nullptr), total_used_size_(0),
          initial_block_capacity_(initial_block_capacity) {
        assert(
            initial_block_capacity != 0 &&
            "Initial block capacity cannot be zero."
        );
        grow_arena(initial_block_capacity);
    }

    // Destructor: Frees all memory blocks owned by the arena.
    void destroy() {
        // Iterate through all blocks and deallocate their memory and metadata
        // Start from the last block allocated
        ArenaBlock* current = current_block_;
        while (current != nullptr) {
            // Keep track to free current block
            ArenaBlock* prev_block = current;
            current = current->next; // Move to the next block in the chain
                                     // (towards head)
            platform_free(prev_block->memory, prev_block->capacity);
            platform_free(prev_block, sizeof(ArenaBlock));
        }
        current_block_ = nullptr;
    }

    // Push: Allocates 'size' bytes from the arena, with specified alignment.
    // Returns a pointer to the allocated memory.
    void* push(usize size, usize alignment = 8) {
        assert(size != 0);
        assert(alignment != 0);
        assert(
            (alignment & (alignment - 1)) == 0 &&
            "Alignment must be a power of two."
        );

        // Calculate aligned pointer and required size including padding
        u8* aligned_ptr_in_block = (u8*)(align_ptr(
            current_block_->memory + current_block_->used,
            alignment
        ));
        usize padded_size = (aligned_ptr_in_block -
                             (current_block_->memory + current_block_->used)) +
                            size;

        // Check if current block has enough space
        if (current_block_->used + padded_size > current_block_->capacity) {
            // Current block is full, grow the arena by allocating a new block
            usize new_block_capacity = std::max(
                initial_block_capacity_,
                size + alignment - 1
            ); // Ensure new block can fit 'size' after alignment
            grow_arena(new_block_capacity);

            // Recalculate aligned pointer for the new block
            aligned_ptr_in_block = (u8*)(align_ptr(
                current_block_->memory + current_block_->used,
                alignment
            ));
            padded_size = (aligned_ptr_in_block -
                           (current_block_->memory + current_block_->used)) +
                          size;
        }

        // Allocate from the current block
        u8* allocated_ptr = aligned_ptr_in_block;
        // Update used bytes
        current_block_->used = (allocated_ptr - current_block_->memory) + size;
        // Accumulate total logical size across blocks
        total_used_size_ += padded_size;
        return allocated_ptr;
    }

    // PushZero: Allocates 'size' bytes and initializes them to zero.
    void* push_zero(usize size, usize alignment = 8) {
        void* ptr = push(size, alignment);
        if (ptr) {
            memset(ptr, 0, size);
        }
        return ptr;
    }

    // Pop: Resets the total allocated size to `new_total_size`.
    // Effectively deallocates all memory pushed after `new_total_size`.
    // This is a "logical" pop; memory within blocks is only marked as unused.
    void pop(usize new_total_size) {
        assert(
            new_total_size < total_used_size_ &&
            "Cannot pop to a position beyond current total used size."
        );

        if (new_total_size == total_used_size_) {
            return; // No change
        }

        // Traverse blocks from newest to oldest to find the block containing
        // new_total_size
        usize remaining_size = new_total_size;
        ArenaBlock* target_block = nullptr;

        // Find the block that contains or comes after new_total_size
        // We traverse *backwards* from current_block_ to logically reset
        ArenaBlock* current = current_block_;
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
            ArenaBlock* temp = current_block_;
            while (temp != target_block) {
                temp->used = 0;
                temp = temp->next;
            }
            current_block_ = target_block; // Update current_block_ to the block
                                           // where new_total_size ends
        } else {                           // new_total_size is 0 or less
            clear();
        }
        total_used_size_ = new_total_size;
    }

    // ArenaClear: Resets the arena, making all its memory available for reuse.
    // Does not free the underlying memory blocks to the OS.
    void clear() {
        if (current_block_) {
            // Reset used space for all blocks
            ArenaBlock* current = current_block_;
            while (current) {
                current->used = 0;
                current = current->next;
            }
            // Point to the first block for new allocations
            current_block_ = get_first_block();
            total_used_size_ = 0;
        }
    }

    // Get current total allocated size across all blocks.
    usize get_total_used_size() const { return total_used_size_; }

    // Helper macro for C-style convenience
    template <typename T>
    T* push_array(usize count, usize alignment = alignof(T)) {
        return (T*)(push(sizeof(T) * count, alignment));
    }

    template <typename T>
    T* push_array_zero(usize count, usize alignment = alignof(T)) {
        return (T*)(push_zero(sizeof(T) * count, alignment));
    }

    template <typename T> T* push_struct(usize alignment = alignof(T)) {
        void* memory = push(sizeof(T), alignment);
        return new (memory) T(); // Constructs T in uninitialized memory
    }

    template <typename T> T* push_struct_zero(usize alignment = alignof(T)) {
        void* memory = push_zero(sizeof(T), alignment);
        return new (memory) T(); // Constructs T in zeroed memory
    }

  private:
    ArenaBlock*
        current_block_; // Pointer to the current active block for allocations
    usize total_used_size_; // Accumulative logical size of all allocations
    usize initial_block_capacity_; // Default capacity for new blocks

    // Internal function to allocate a new memory block and add it to the chain.
    void grow_arena(usize min_capacity) {
        usize actual_capacity = std::max(min_capacity, initial_block_capacity_);

        ArenaBlock* new_block = (ArenaBlock*)platform_alloc(
            sizeof(ArenaBlock)
        ); // Allocate metadata for the new block
        new_block->memory =
            (u8*)platform_alloc(actual_capacity); // Allocate raw memory
        new_block->capacity = actual_capacity;
        new_block->used = 0;
        new_block->next = current_block_; // Link new block to the previous
                                          // (making it the head)

        current_block_ =
            new_block; // New block becomes the current active block
    }

    // Helper to get the very first block in the chain (oldest block)
    ArenaBlock* get_first_block() {
        ArenaBlock* current = current_block_;
        if (!current) return nullptr;
        while (current->next) {
            current = current->next;
        }
        return current;
    }
};
