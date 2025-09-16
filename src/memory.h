#pragma once
#include "types.h"

#ifdef _WIN32
    #include <windows.h>
    
    void* platform_alloc(usize size) {
        return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    }
    
    void platform_free(void* ptr, usize size) {
        VirtualFree(ptr, 0, MEM_RELEASE);
    }
    
#elif defined(__linux__) || defined(__APPLE__)
    #include <sys/mman.h>
    
    void* platform_alloc(usize size) {
        void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, 
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        return (ptr == MAP_FAILED) ? nullptr : ptr;
    }
    
    void platform_free(void* ptr, usize size) {
        munmap(ptr, size);
    }
#endif