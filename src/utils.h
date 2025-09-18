#pragma once

#include <utility>

#define BIT(x) 1 << (x)
#define KB(x) ((usize)1024 * x)
#define MB(x) ((usize)1024 * KB(x))
#define GB(x) ((usize)1024 * MB(x))

#define unreachable __builtin_unreachable()

template <typename F> struct Defer {
    Defer(F f) : f(f) {}
    ~Defer() { f(); }
    F f;
};

template <typename F> Defer<F> makeDefer(F f) { return Defer<F>(f); };

#define __defer(line) defer_##line
#define _defer(line) __defer(line)

struct defer_dummy {};
template <typename F> Defer<F> operator+(defer_dummy, F&& f) {
    return makeDefer<F>(std::forward<F>(f));
}

#define defer auto _defer(__LINE__) = defer_dummy() + [&]()

#ifdef _WIN32
#define export extern "C" __declspec(dllexport)
#elif defined(__linux__) || defined(__APPLE__)
#ifdef __cplusplus
#define export extern "C" __attribute__((visibility("default")))
#else
#define export __attribute__((visibility("default")))
#endif
#endif

#ifdef _WIN32
#define DYNLIB(name) name ".dll"
#elif defined(__linux__)
#define DYNLIB(name) name ".so"
#elif defined(__APPLE__)
#define DYNLIB(name) name ".dylib"
#endif

#ifdef DEBUG
#define debug_mode true
#else
#define debug_mode false
#endif
