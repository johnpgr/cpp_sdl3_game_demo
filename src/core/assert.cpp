#include "core/assert.h"
#include <SDL3/SDL_log.h>

#ifdef __APPLE__
#include <execinfo.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#include <dbghelp.h>
#elif defined(__linux__)
#include <execinfo.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

bool is_debugger_attached() {
#ifdef __APPLE__
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};
    struct kinfo_proc info;
    size_t size = sizeof(info);

    if (sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0) ==
        0) {
        return (info.kp_proc.p_flag & P_TRACED) != 0;
    }
    return false;
#elif defined(_WIN32)
    return IsDebuggerPresent();
#elif defined(__linux__)
    static bool cached_result = false;
    static bool cached = false;

    if (!cached) {
        pid_t pid = fork();
        if (pid == -1) {
            cached_result = false;
        } else if (pid == 0) {
            if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) == -1) {
                exit(1);
            }
            exit(0);
        } else {
            int status;
            waitpid(pid, &status, 0);
            cached_result = WEXITSTATUS(status) == 0;
        }
        cached = true;
    }
    return cached_result;
#else
    return false;
#endif
}

void print_stack_trace() {
#if defined(__APPLE__) || defined(__linux__)
    void* array[10];
    size_t size = backtrace(array, 10);
    char** strings = backtrace_symbols(array, size);

    SDL_Log("Stack trace:");
    for (size_t i = 0; i < size; i++) {
        SDL_Log("  %s", strings[i]);
    }
    free(strings);
#elif defined(_WIN32)
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, TRUE);

    void* stack[10];
    WORD frames = CaptureStackBackTrace(0, 10, stack, nullptr);

    SDL_Log("Stack trace:");
    for (WORD i = 0; i < frames; i++) {
        SDL_Log("  Frame %d: %p", i, stack[i]);
    }

    SymCleanup(process);
#else
    SDL_Log("Stack trace not available on this platform");
#endif
}
