#!/bin/bash

set -e  # Exit on any error

# Project configuration
PROJECT_NAME="game"
SRC_DIR="src"
BUILD_DIR="build"
ASSETS_DIR="assets"
SHADER_SOURCE_DIR="$ASSETS_DIR/shaders/source"
SHADER_OUT_DIR="$ASSETS_DIR/shaders/compiled"

# Compiler settings
CXX="zig c++"
CXXFLAGS="-std=c++23 -Wall -Wextra -Wno-address-of-temporary -Wno-missing-designated-field-initializers"
DEBUG_FLAGS="-g -DDEBUG"
RELEASE_FLAGS="-DNDEBUG -O3"

# Build mode from environment variable, default to debug
BUILD_MODE=${BUILD_MODE:-debug}

# Platform detection
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macOS"
    LIBRARY_EXT=".dylib"
    SHARED_FLAGS="-shared -fPIC -undefined dynamic_lookup"
else
    PLATFORM="Unix"
    LIBRARY_EXT=".so"
    SHARED_FLAGS="-shared -fPIC"
fi

# SDL configuration using pkg-config with fallbacks
SDL3_INCLUDE=$(pkg-config --cflags sdl3 2>/dev/null || echo "-I/usr/local/include/SDL3 -I/usr/include/SDL3")
SDL3_LIBS=$(pkg-config --libs sdl3 2>/dev/null || echo "-lSDL3")

SDL3_IMAGE_INCLUDE=$(pkg-config --cflags SDL3_image 2>/dev/null || echo "-I/usr/local/include/SDL3 -I/usr/include/SDL3")
SDL3_IMAGE_LIBS=$(pkg-config --libs SDL3_image 2>/dev/null || echo "-lSDL3_image")

SDL3_TTF_INCLUDE=$(pkg-config --cflags SDL3_ttf 2>/dev/null || echo "-I/usr/local/include/SDL3 -I/usr/include/SDL3")
SDL3_TTF_LIBS=$(pkg-config --libs SDL3_ttf 2>/dev/null || echo "-lSDL3_ttf")

SDL_INCLUDE="$SDL3_INCLUDE $SDL3_IMAGE_INCLUDE $SDL3_TTF_INCLUDE"
SDL_LIBS="$SDL3_LIBS $SDL3_IMAGE_LIBS $SDL3_TTF_LIBS"
DL_LIBS="-ldl"

# Build mode setup
if [[ "$BUILD_MODE" == "release" ]]; then
    CXXFLAGS="$CXXFLAGS $RELEASE_FLAGS"
    BUILD_SUFFIX="release"
else
    CXXFLAGS="$CXXFLAGS $DEBUG_FLAGS"
    BUILD_SUFFIX="debug"
fi

OBJ_DIR="$BUILD_DIR/$BUILD_SUFFIX/obj"
BIN_DIR="$BUILD_DIR/$BUILD_SUFFIX/bin"
EXECUTABLE="$BIN_DIR/$PROJECT_NAME"
GAME_LIBRARY="$BIN_DIR/libgame$LIBRARY_EXT"

echo "Building in $BUILD_SUFFIX mode..."

# Create directories
mkdir -p "$OBJ_DIR" "$BIN_DIR" "$SHADER_OUT_DIR"

# Compile shaders if shadercross is available
if command -v shadercross &> /dev/null && [[ -d "$SHADER_SOURCE_DIR" ]]; then
    echo "Compiling shaders..."
    
    # Compile vertex shaders
    for shader in "$SHADER_SOURCE_DIR"/*.vert.hlsl; do
        [[ -f "$shader" ]] || continue
        basename=$(basename "$shader" .vert.hlsl)
        echo "  Compiling vertex shader: $basename"
        shadercross "$shader" -o "$SHADER_OUT_DIR/$basename.vert.spv" || true
        shadercross "$shader" -o "$SHADER_OUT_DIR/$basename.vert.msl" || true
        shadercross "$shader" -o "$SHADER_OUT_DIR/$basename.vert.dxil" || true
    done
    
    # Compile fragment shaders
    for shader in "$SHADER_SOURCE_DIR"/*.frag.hlsl; do
        [[ -f "$shader" ]] || continue
        basename=$(basename "$shader" .frag.hlsl)
        echo "  Compiling fragment shader: $basename"
        shadercross "$shader" -o "$SHADER_OUT_DIR/$basename.frag.spv" || true
        shadercross "$shader" -o "$SHADER_OUT_DIR/$basename.frag.msl" || true
        shadercross "$shader" -o "$SHADER_OUT_DIR/$basename.frag.dxil" || true
    done
    
    # Compile compute shaders
    for shader in "$SHADER_SOURCE_DIR"/*.comp.hlsl; do
        [[ -f "$shader" ]] || continue
        basename=$(basename "$shader" .comp.hlsl)
        echo "  Compiling compute shader: $basename"
        shadercross "$shader" -o "$SHADER_OUT_DIR/$basename.comp.spv" || true
        shadercross "$shader" -o "$SHADER_OUT_DIR/$basename.comp.msl" || true
        shadercross "$shader" -o "$SHADER_OUT_DIR/$basename.comp.dxil" || true
    done
else
    if ! command -v shadercross &> /dev/null; then
        echo "shadercross not found, skipping shader compilation"
    else
        echo "Shader source directory not found: $SHADER_SOURCE_DIR"
    fi
fi

# Copy assets
if [[ -d "$ASSETS_DIR" ]]; then
    echo "Copying assets..."
    cp -r "$ASSETS_DIR" "$BIN_DIR/"
else
    echo "Warning: $ASSETS_DIR directory not found, skipping asset copy"
fi

# Compile source files
INCLUDES="-I$SRC_DIR $SDL_INCLUDE"

echo "Compiling sources..."
$CXX $CXXFLAGS $INCLUDES -c "$SRC_DIR/main.cpp" -o "$OBJ_DIR/main.o"
$CXX $CXXFLAGS $INCLUDES -c "$SRC_DIR/game.cpp" -o "$OBJ_DIR/game.o"

# Link executables
echo "Linking targets..."

# Try to link main executable, but don't fail the script if it's locked
if $CXX $CXXFLAGS "$OBJ_DIR/main.o" -o "$EXECUTABLE" $SDL_LIBS $DL_LIBS 2>/dev/null; then
    echo "Main executable: $EXECUTABLE"
else
    echo "Warning: Failed to link main executable (file may be locked by debugger)"
fi

# Always try to build the game library
$CXX $CXXFLAGS $SHARED_FLAGS "$OBJ_DIR/game.o" -o "$GAME_LIBRARY" $SDL_LIBS $DL_LIBS

echo "Build complete!"
echo "Game library: $GAME_LIBRARY"
