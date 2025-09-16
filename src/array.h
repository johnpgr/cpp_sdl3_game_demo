#pragma once

#include "types.h"

template <typename T, usize N> struct Array {
    usize capacity;
    usize size;
    T items[N];

    T& operator[](usize idx);
    usize push(T item);
    T& pop();
    T& remove_at(usize idx);
    usize insert_at(usize idx, T item);
    void clear();
    bool is_full();
};
