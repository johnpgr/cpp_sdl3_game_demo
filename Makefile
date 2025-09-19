# Project configuration
PROJECT_NAME := game
SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
DEPS_DIR := $(BUILD_DIR)/deps
ASSETS_DIR := assets

# Shader configuration
SHADER_SOURCE_DIR := $(ASSETS_DIR)/shaders/source
SHADER_OUT_DIR := $(ASSETS_DIR)/shaders/compiled
SHADERCROSS := shadercross

# Compiler settings
CXX := clang++
CXXFLAGS := -std=c++23 -Wall -Wextra -Wno-address-of-temporary -Wno-missing-designated-field-initializers
DEBUG_FLAGS := -g -DDEBUG
RELEASE_FLAGS := -DNDEBUG -O3

RM := rm -f
RMDIR := rm -rf
MKDIR := mkdir -p
COPY := cp
COPYR := cp -r

# Platform detection
ifeq ($(OS),Windows_NT)
    PLATFORM := Windows
    EXECUTABLE := $(PROJECT_NAME).exe
    LIBRARY_EXT := .dll
	LDFLAGS := -fuse-ld=lld -Wl,/SUBSYSTEM:WINDOWS -Wl,/NOIMPLIB
    SHARED_FLAGS := -shared
else
    PLATFORM := Unix
    EXECUTABLE := $(PROJECT_NAME)
    LIBRARY_EXT := .so
    SHARED_FLAGS := -shared -fPIC
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        PLATFORM := macOS
        LIBRARY_EXT := .dylib
        SHARED_FLAGS := -shared -fPIC -undefined dynamic_lookup
    endif
endif

# SDL3, SDL3_image and SDL3_ttf configuration
ifeq ($(PLATFORM),Windows)
    # Windows: Assume SDL3 is installed in a standard location or set SDL3_DIR
    ifdef SDL3_DIR
        SDL3_INCLUDE := -I$(SDL3_DIR)/include
        SDL3_LIBS := -L$(SDL3_DIR)/lib -lSDL3
        # SDL3_image is typically in the same directory structure
        SDL3_IMAGE_INCLUDE := -I$(SDL3_DIR)/include
        SDL3_IMAGE_LIBS := -L$(SDL3_DIR)/lib -lSDL3_image
        # SDL3_ttf is typically in the same directory structure
        SDL3_TTF_INCLUDE := -I$(SDL3_DIR)/include
        SDL3_TTF_LIBS := -L$(SDL3_DIR)/lib -lSDL3_ttf
    else
        # Default Windows paths - adjust as needed
        SDL3_INCLUDE := -IC:/SDKs/SDL3/include
        SDL3_LIBS := -LC:/SDKs/SDL3/lib -lSDL3
        SDL3_IMAGE_INCLUDE := -IC:/SDKs/SDL3_image/include
        SDL3_IMAGE_LIBS := -LC:/SDKs/SDL3_image/lib -lSDL3_image
        SDL3_TTF_INCLUDE := -IC:/SDKs/SDL3_ttf/include
        SDL3_TTF_LIBS := -LC:/SDKs/SDL3_ttf/lib -lSDL3_ttf
    endif
else
    # Unix: Use pkg-config
    SDL3_INCLUDE := $(shell pkg-config --cflags sdl3 2>/dev/null)
    SDL3_LIBS := $(shell pkg-config --libs sdl3 2>/dev/null)
    SDL3_IMAGE_INCLUDE := $(shell pkg-config --cflags SDL3_image 2>/dev/null)
    SDL3_IMAGE_LIBS := $(shell pkg-config --libs SDL3_image 2>/dev/null)
    SDL3_TTF_INCLUDE := $(shell pkg-config --cflags SDL3_ttf 2>/dev/null)
    SDL3_TTF_LIBS := $(shell pkg-config --libs SDL3_ttf 2>/dev/null)

    # Fallback if pkg-config fails
    ifeq ($(SDL3_INCLUDE),)
        SDL3_INCLUDE := -I/usr/local/include/SDL3 -I/usr/include/SDL3
    endif
    ifeq ($(SDL3_LIBS),)
        SDL3_LIBS := -lSDL3
    endif

    ifeq ($(SDL3_IMAGE_INCLUDE),)
        SDL3_IMAGE_INCLUDE := -I/usr/local/include/SDL3 -I/usr/include/SDL3
    endif
    ifeq ($(SDL3_IMAGE_LIBS),)
        SDL3_IMAGE_LIBS := -lSDL3_image
    endif

    ifeq ($(SDL3_TTF_INCLUDE),)
        SDL3_TTF_INCLUDE := -I/usr/local/include/SDL3 -I/usr/include/SDL3
    endif
    ifeq ($(SDL3_TTF_LIBS),)
        SDL3_TTF_LIBS := -lSDL3_ttf
    endif
endif

# Combine SDL libraries
SDL_INCLUDE := $(SDL3_INCLUDE) $(SDL3_IMAGE_INCLUDE) $(SDL3_TTF_INCLUDE)
SDL_LIBS := $(SDL3_LIBS) $(SDL3_IMAGE_LIBS) $(SDL3_TTF_LIBS)

# Include directories
INCLUDES := -I$(SRC_DIR) $(SDL_INCLUDE)

# Build modes
MODE ?= debug
ifeq ($(MODE),debug)
    CXXFLAGS += $(DEBUG_FLAGS)
    BUILD_SUFFIX := debug
else
    CXXFLAGS += $(RELEASE_FLAGS)
    BUILD_SUFFIX := release
endif

# Update directories with build mode
OBJ_DIR := $(BUILD_DIR)/$(BUILD_SUFFIX)/obj
DEPS_DIR := $(BUILD_DIR)/$(BUILD_SUFFIX)/deps
BIN_DIR := $(BUILD_DIR)/$(BUILD_SUFFIX)/bin

# Target files
MAIN_SOURCE := $(SRC_DIR)/main.cpp
GAME_SOURCE := $(SRC_DIR)/game.cpp
MAIN_TARGET := $(BIN_DIR)/$(EXECUTABLE)
GAME_TARGET := $(BIN_DIR)/libgame$(LIBRARY_EXT)
MAIN_PDB := $(BIN_DIR)/$(PROJECT_NAME).pdb
GAME_PDB := $(BIN_DIR)/libgame.pdb

# Object files
MAIN_OBJECT := $(OBJ_DIR)/main.o
GAME_OBJECT := $(OBJ_DIR)/game.o
MAIN_DEPENDS := $(DEPS_DIR)/main.d
GAME_DEPENDS := $(DEPS_DIR)/game.d

# Platform-specific dynamic loading libraries
ifeq ($(PLATFORM),Windows)
    DL_LIBS := -ldbghelp
else
    DL_LIBS := -ldl
endif

# Shader source files
VERT_SHADERS := $(wildcard $(SHADER_SOURCE_DIR)/*.vert.hlsl)
FRAG_SHADERS := $(wildcard $(SHADER_SOURCE_DIR)/*.frag.hlsl)
COMP_SHADERS := $(wildcard $(SHADER_SOURCE_DIR)/*.comp.hlsl)

# Compiled shader files
VERT_SPV := $(patsubst $(SHADER_SOURCE_DIR)/%.vert.hlsl,$(SHADER_OUT_DIR)/%.vert.spv,$(VERT_SHADERS))
VERT_MSL := $(patsubst $(SHADER_SOURCE_DIR)/%.vert.hlsl,$(SHADER_OUT_DIR)/%.vert.msl,$(VERT_SHADERS))
VERT_DXIL := $(patsubst $(SHADER_SOURCE_DIR)/%.vert.hlsl,$(SHADER_OUT_DIR)/%.vert.dxil,$(VERT_SHADERS))

FRAG_SPV := $(patsubst $(SHADER_SOURCE_DIR)/%.frag.hlsl,$(SHADER_OUT_DIR)/%.frag.spv,$(FRAG_SHADERS))
FRAG_MSL := $(patsubst $(SHADER_SOURCE_DIR)/%.frag.hlsl,$(SHADER_OUT_DIR)/%.frag.msl,$(FRAG_SHADERS))
FRAG_DXIL := $(patsubst $(SHADER_SOURCE_DIR)/%.frag.hlsl,$(SHADER_OUT_DIR)/%.frag.dxil,$(FRAG_SHADERS))

COMP_SPV := $(patsubst $(SHADER_SOURCE_DIR)/%.comp.hlsl,$(SHADER_OUT_DIR)/%.comp.spv,$(COMP_SHADERS))
COMP_MSL := $(patsubst $(SHADER_SOURCE_DIR)/%.comp.hlsl,$(SHADER_OUT_DIR)/%.comp.msl,$(COMP_SHADERS))
COMP_DXIL := $(patsubst $(SHADER_SOURCE_DIR)/%.comp.hlsl,$(SHADER_OUT_DIR)/%.comp.dxil,$(COMP_SHADERS))

# All compiled shaders
ALL_COMPILED_SHADERS := $(VERT_SPV) $(VERT_MSL) $(VERT_DXIL) $(FRAG_SPV) $(FRAG_MSL) $(FRAG_DXIL) $(COMP_SPV) $(COMP_MSL) $(COMP_DXIL)

# Phony targets
.PHONY: all clean debug release help dirs shaders clean-shaders copy-assets

# Default target
all: shaders compile-shaders copy-assets $(MAIN_TARGET) $(GAME_TARGET)

# Debug build
debug:
	$(MAKE) MODE=debug

# Release build
release:
	$(MAKE) MODE=release

# Create necessary directories
dirs:
	-$(MKDIR) $(OBJ_DIR)
	-$(MKDIR) $(DEPS_DIR)
	-$(MKDIR) $(BIN_DIR)

# Create shader directories
shader-dirs:
	-$(MKDIR) $(SHADER_OUT_DIR)

# Copy assets to bin directory after build
copy-assets: | dirs
	@echo "Copying assets to $(BIN_DIR)..."
ifeq ($(wildcard $(ASSETS_DIR)),$(ASSETS_DIR))
ifeq ($(PLATFORM),Windows)
	$(COPYR) $(ASSETS_DIR) $(BIN_DIR)/
else
	$(COPYR) $(ASSETS_DIR) $(BIN_DIR)/
endif
else
	@echo "Warning: $(ASSETS_DIR) directory not found, skipping asset copy"
endif

# Check if shadercross is available and compile shaders
shaders: shader-dirs

# Compile all shaders (only runs if shadercross is available)
compile-shaders: $(ALL_COMPILED_SHADERS)

# Vertex shader compilation rules
$(SHADER_OUT_DIR)/%.vert.spv: $(SHADER_SOURCE_DIR)/%.vert.hlsl | shader-dirs
	@echo "Compiling vertex shader: $< -> $@"
	-$(SHADERCROSS) $< -o $@

$(SHADER_OUT_DIR)/%.vert.msl: $(SHADER_SOURCE_DIR)/%.vert.hlsl | shader-dirs
	@echo "Compiling vertex shader: $< -> $@"
	-$(SHADERCROSS) $< -o $@

$(SHADER_OUT_DIR)/%.vert.dxil: $(SHADER_SOURCE_DIR)/%.vert.hlsl | shader-dirs
	@echo "Compiling vertex shader: $< -> $@"
	-$(SHADERCROSS) $< -o $@

# Fragment shader compilation rules
$(SHADER_OUT_DIR)/%.frag.spv: $(SHADER_SOURCE_DIR)/%.frag.hlsl | shader-dirs
	@echo "Compiling fragment shader: $< -> $@"
	-$(SHADERCROSS) $< -o $@

$(SHADER_OUT_DIR)/%.frag.msl: $(SHADER_SOURCE_DIR)/%.frag.hlsl | shader-dirs
	@echo "Compiling fragment shader: $< -> $@"
	-$(SHADERCROSS) $< -o $@

$(SHADER_OUT_DIR)/%.frag.dxil: $(SHADER_SOURCE_DIR)/%.frag.hlsl | shader-dirs
	@echo "Compiling fragment shader: $< -> $@"
	-$(SHADERCROSS) $< -o $@

# Compute shader compilation rules
$(SHADER_OUT_DIR)/%.comp.spv: $(SHADER_SOURCE_DIR)/%.comp.hlsl | shader-dirs
	@echo "Compiling compute shader: $< -> $@"
	-$(SHADERCROSS) $< -o $@

$(SHADER_OUT_DIR)/%.comp.msl: $(SHADER_SOURCE_DIR)/%.comp.hlsl | shader-dirs
	@echo "Compiling compute shader: $< -> $@"
	-$(SHADERCROSS) $< -o $@

$(SHADER_OUT_DIR)/%.comp.dxil: $(SHADER_SOURCE_DIR)/%.comp.hlsl | shader-dirs
	@echo "Compiling compute shader: $< -> $@"
	-$(SHADERCROSS) $< -o $@

# Link the main executable
$(MAIN_TARGET): $(MAIN_OBJECT) | dirs
	@echo "Linking $(MAIN_TARGET)..."
ifeq ($(PLATFORM),Windows)
	$(CXX) $(CXXFLAGS) $(MAIN_OBJECT) -o $@ $(LDFLAGS) -Wl,/PDB:$(MAIN_PDB) $(SDL_LIBS) $(DL_LIBS)
else
	$(CXX) $(CXXFLAGS) $(MAIN_OBJECT) -o $@ $(LDFLAGS) $(SDL_LIBS) $(DL_LIBS)
endif
	@echo "Main executable complete: $(MAIN_TARGET)"

# Build the game library
$(GAME_TARGET): $(GAME_OBJECT) | dirs
	@echo "Linking $(GAME_TARGET)..."
ifeq ($(PLATFORM),Windows)
	$(CXX) $(CXXFLAGS) $(SHARED_FLAGS) $(GAME_OBJECT) -o $@ $(LDFLAGS) -Wl,/PDB:$(GAME_PDB) $(SDL_LIBS) $(DL_LIBS)
else
	$(CXX) $(CXXFLAGS) $(SHARED_FLAGS) $(GAME_OBJECT) -o $@ $(LDFLAGS) $(SDL_LIBS) $(DL_LIBS)
endif
	@echo "Game library complete: $(GAME_TARGET)"

# Compile main.cpp
$(MAIN_OBJECT): $(MAIN_SOURCE) | dirs
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -MF $(MAIN_DEPENDS) -c $< -o $@

# Compile game.cpp
$(GAME_OBJECT): $(GAME_SOURCE) | dirs
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -MF $(GAME_DEPENDS) -c $< -o $@

# Include dependency files
-include $(MAIN_DEPENDS) $(GAME_DEPENDS)

# Clean compiled shaders
clean-shaders:
	@echo "Cleaning compiled shaders..."
	-$(RM) $(SHADER_OUT_DIR)/*

# Clean build files
clean:
	@echo "Cleaning build directory..."
	-$(RMDIR) $(BUILD_DIR)

# Clean everything including shaders
clean-all: clean clean-shaders

# Help target
help:
	@echo "Available targets:"
	@echo "  all         - Build the project (shaders + main executable + game library + copy assets)"
	@echo "  debug       - Build in debug mode"
	@echo "  release     - Build in release mode"
	@echo "  shaders     - Compile shaders (if shadercross is available)"
	@echo "  clean       - Remove all build files"
	@echo "  clean-shaders - Remove compiled shaders"
	@echo "  clean-all   - Remove build files and compiled shaders"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Dependencies:"
	@echo "  SDL3, SDL3_image, SDL3_ttf (configure paths with SDL3_DIR on Windows)"
	@echo "  shadercross (optional - for shader compilation)"
	@echo ""
	@echo "Shader structure:"
	@echo "  $(SHADER_SOURCE_DIR)/    - Source shaders (.vert.hlsl, .frag.hlsl, .comp.hlsl)"
	@echo "  $(SHADER_OUT_DIR)/      - Compiled shaders (.spv, .msl, .dxil)"
	@echo ""
	@echo "Build structure:"
	@echo "  $(BUILD_DIR)/$(BUILD_SUFFIX)/obj/     - Object files (.o)"
	@echo "  $(BUILD_DIR)/$(BUILD_SUFFIX)/deps/    - Dependency files (.d)"
	@echo "  $(BUILD_DIR)/$(BUILD_SUFFIX)/bin/     - Built binaries and assets"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Debug build with shaders and assets"
	@echo "  make release            # Release build with shaders and assets"
	@echo "  make MODE=release       # Release build (alternative)"
	@echo "  make shaders            # Compile shaders only"
