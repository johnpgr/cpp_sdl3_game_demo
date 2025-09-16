#include "memory.h"

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/mman.h>
#endif

static void* platform_alloc(usize size) {
#ifdef _WIN32
    return VirtualAlloc(
        nullptr,
        size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );
#elif defined(__linux__) || defined(__APPLE__)
    void* ptr = mmap(
        nullptr,
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );
    return (ptr == MAP_FAILED) ? nullptr : ptr;
#else
    return nullptr;
#endif
}

static void platform_free(void* ptr, usize size) {
#ifdef _WIN32
    VirtualFree(ptr, 0, MEM_RELEASE);
#elif defined(__linux__) || defined(__APPLE__)
    munmap(ptr, size);
#endif
}
