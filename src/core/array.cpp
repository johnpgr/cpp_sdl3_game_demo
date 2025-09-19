#include "core/array.h"
#include "core/assert.h"

template <typename T, usize N> T& Array<T, N>::operator[](usize idx) {
    DEBUG_ASSERT(idx < size, "Array access out of bounds");
    return items[idx];
}

template <typename T, usize N> usize Array<T, N>::push(T item) {
    DEBUG_ASSERT(size < N, "Array full");
    items[size] = item;
    return size++;
}

template <typename T, usize N> T& Array<T, N>::pop() {
    DEBUG_ASSERT(size > 0, "Array is empty");
    return items[--size];
}

template <typename T, usize N> T& Array<T, N>::remove_at(usize idx) {
    DEBUG_ASSERT(idx < size, "Array access out of bounds");

    T& removed = items[idx];
    for (usize i = idx; i < size - 1; i++) {
        items[i] = items[i + 1];
    }
    size--;

    return removed;
}

template <typename T, usize N> usize Array<T, N>::insert_at(usize idx, T item) {
    DEBUG_ASSERT(idx < size, "Array access out of bounds");
    DEBUG_ASSERT(size < N, "Array is full");

    for (usize i = size; i > idx; i--) {
        items[i] = items[i - 1];
    }

    items[idx] = item;
    return size++;
}

template <typename T, usize N> void Array<T, N>::clear() { size = 0; }

template <typename T, usize N> bool Array<T, N>::is_full() { return size == N; }
template <typename T, usize N> bool Array<T, N>::is_empty() { return size == 0; }
