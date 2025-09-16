#pragma once

#include "types.h"
#include <assert.h>

template <typename T, usize N> struct Array {
    usize capacity{N};
    usize size{};
    T items[N]{};

    T& operator[](usize idx) {
        assert(idx < size && "Array access out of bounds");
        return items[idx];
    }

    usize push(T item) {
        assert(size < capacity && "Array full");
        items[size] = item;
        return size++;
    }

    T& pop() {
        assert(size > 0 && "Array is empty");
        return items[--size];
    }

    T& remove_at(usize idx) {
        assert(idx < size && "Array access out of bounds");

        T& removed = items[idx];
        for (usize i = idx; i < size - 1; i++) {
            items[i] = items[i + 1];
        }
        size--;

        return removed;
    }

    usize insert_at(usize idx, T item) {
        assert(idx < size && "Array access out of bounds");
        assert(size < capacity && "Array is full");

        for (usize i = size; i > idx; i--) {
            items[i] = items[i - 1];
        }

        items[idx] = item;
        return size++;
    }

    void clear() { size = 0; }

    bool is_full() { return size == N; }
};
