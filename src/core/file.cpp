#include "core/file.h"
#include <SDL3/SDL.h>

/**
 * @brief Gets the last modification timestamp of a file.
 * @param path The path to the file.
 * @return A 64-bit integer representing the timestamp (seconds since epoch), or 0 on error.
 */
u64 file_get_timestamp(const char* path) {
    SDL_PathInfo info{};
    if (SDL_GetPathInfo(path, &info)) {
        return info.modify_time;
    }
    SDL_Log("Could not get timestamp for '%s': %s", path, SDL_GetError());
    return 0;
}

/**
 * @brief Checks if a file or directory exists at the given path.
 * @param path The path to check.
 * @return True if the file exists, false otherwise.
 */
bool file_exists(const char* path) {
    return SDL_GetPathInfo(path, nullptr);
}

/**
 * @brief Gets the size of a file in bytes.
 * @param path The path to the file.
 * @return The size of the file as a 'usize', or 0 if the file doesn't exist or is empty.
 */
usize file_get_size(const char* path) {
    SDL_PathInfo info;
    if (SDL_GetPathInfo(path, &info)) {
        return (usize)info.size;
    }
    return 0;
}

/**
 * @brief Reads the entire content of a file into a buffer allocated from an arena.
 * @param arena A pointer to the memory arena to allocate from.
 * @param path The path to the file.
 * @return A pointer to a null-terminated buffer with the file content, or nullptr on failure.
 */
char* read_entire_file(Arena* arena, const char* path) {
    SDL_IOStream* file = SDL_IOFromFile(path, "rb");
    if (!file) {
        SDL_Log("Failed to open file for reading '%s': %s", path, SDL_GetError());
        return nullptr;
    }

    Sint64 file_size_s64 = SDL_GetIOSize(file);
    if (file_size_s64 < 0) {
        SDL_Log("Failed to get size of file '%s': %s", path, SDL_GetError());
        SDL_CloseIO(file);
        return nullptr;
    }
    usize file_size = (usize)file_size_s64;

    // Allocate space for the file content plus a null terminator.
    char* buffer = (char*)arena->push(file_size + 1);
    if (!buffer) {
        SDL_Log("Failed to allocate %zu bytes for file '%s'", file_size + 1, path);
        SDL_CloseIO(file);
        return nullptr;
    }

    usize bytes_read = SDL_ReadIO(file, buffer, file_size);
    SDL_CloseIO(file);

    if (bytes_read != file_size) {
        SDL_Log("Failed to read entire file '%s'. Expected %zu, got %zu", path, file_size, bytes_read);
        return nullptr;
    }

    buffer[file_size] = '\0';
    return buffer;
}

/**
 * @brief Writes data to a file, overwriting it if it exists or creating it if it doesn't.
 * @param path The path to the file to write to.
 * @param data A pointer to the data to be written.
 * @param size The number of bytes to write.
 */
void write_file(const char* path, const char* data, usize size) {
    SDL_IOStream* file = SDL_IOFromFile(path, "wb");
    if (!file) {
        SDL_Log("Failed to open file for writing '%s': %s", path, SDL_GetError());
        return;
    }

    usize bytes_written = SDL_WriteIO(file, data, size);
    if (bytes_written != size) {
        SDL_Log("Failed to write entire buffer to file '%s'. Expected %zu, got %zu", path, size, bytes_written);
    }

    SDL_CloseIO(file);
}

/**
 * @brief Copies a file from a source path to a destination path.
 * @param arena A pointer to a memory arena used for the temporary read buffer.
 * @param src_path The source file path.
 * @param dst_path The destination file path.
 * @return True on success, false on failure.
 */
bool copy_file(Arena* arena, const char* src_path, const char* dst_path) {
    usize size = file_get_size(src_path);
    // Check if the source exists and is not an empty file we can't read.
    if (size == 0 && !file_exists(src_path)) {
        SDL_Log("Source file for copy does not exist: '%s'", src_path);
        return false;
    }
    
    // An empty file can be "copied" by creating an empty destination file.
    if (size == 0) {
        write_file(dst_path, "", 0);
        return file_exists(dst_path);
    }

    char* data = read_entire_file(arena, src_path);
    if (!data) {
        return false;
    }

    write_file(dst_path, data, size);

    // Verify the copy was successful by checking the destination file's size.
    if (file_get_size(dst_path) != size) {
        SDL_Log("Copy failed: Destination file size mismatch for '%s'", dst_path);
        return false;
    }

    return true;
}
