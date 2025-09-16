# Project configuration
PROJECT_NAME := game
SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
DEPS_DIR := $(BUILD_DIR)/deps
DIST_DIR := dist
ASSETS_DIR := assets

# Compiler settings
CXX := clang++
CXXFLAGS := -std=c++23 -Wall -Wextra -O2
DEBUG_FLAGS := -g -DDEBUG -O0
RELEASE_FLAGS := -DNDEBUG -O3

# Platform detection
ifeq ($(OS),Windows_NT)
    PLATFORM := Windows
    EXECUTABLE := $(PROJECT_NAME).exe
    LIBRARY_EXT := .dll
    RM := del /Q
    MKDIR := mkdir
    COPY := copy
    COPYR := xcopy /E /I /Y
    PATH_SEP := \\
    # Windows-specific flags
    LDFLAGS += -static-libgcc -static-libstdc++
    SHARED_FLAGS := -shared
else
    PLATFORM := Unix
    EXECUTABLE := $(PROJECT_NAME)
    LIBRARY_EXT := .so
    RM := rm -f
    MKDIR := mkdir -p
    COPY := cp
    COPYR := cp -r
    PATH_SEP := /
    SHARED_FLAGS := -shared -fPIC
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        PLATFORM := macOS
        LIBRARY_EXT := .dylib
        SHARED_FLAGS := -shared -fPIC -undefined dynamic_lookup
    endif
endif

# SDL3 configuration
ifeq ($(PLATFORM),Windows)
    # Windows: Assume SDL3 is installed in a standard location or set SDL3_DIR
    ifdef SDL3_DIR
        SDL3_INCLUDE := -I$(SDL3_DIR)/include
        SDL3_LIBS := -L$(SDL3_DIR)/lib -lSDL3
    else
        # Default Windows paths - adjust as needed
        SDL3_INCLUDE := -IC:/SDL3/include
        SDL3_LIBS := -LC:/SDL3/lib -lSDL3
    endif
    # Windows system libraries
else
    # Unix: Use pkg-config
    SDL3_INCLUDE := $(shell pkg-config --cflags sdl3 2>/dev/null)
    SDL3_LIBS := $(shell pkg-config --libs sdl3 2>/dev/null)
    
    # Fallback if pkg-config fails
    ifeq ($(SDL3_INCLUDE),)
        SDL3_INCLUDE := -I/usr/local/include/SDL3 -I/usr/include/SDL3
    endif
    ifeq ($(SDL3_LIBS),)
        SDL3_LIBS := -lSDL3
    endif
endif

# Include directories
INCLUDES := -I$(SRC_DIR) $(SDL3_INCLUDE)

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

# Object files
MAIN_OBJECT := $(OBJ_DIR)/main.o
GAME_OBJECT := $(OBJ_DIR)/game.o
MAIN_DEPENDS := $(DEPS_DIR)/main.d
GAME_DEPENDS := $(DEPS_DIR)/game.d

# Platform-specific dynamic loading libraries
ifeq ($(PLATFORM),Windows)
    DL_LIBS := 
else
    DL_LIBS := -ldl
endif

# Phony targets
.PHONY: all clean debug release install help dirs

# Default target
all: $(MAIN_TARGET) $(GAME_TARGET)

# Debug build
debug:
	$(MAKE) MODE=debug

# Release build
release:
	$(MAKE) MODE=release

# Create necessary directories
dirs:
	$(MKDIR) $(OBJ_DIR)
	$(MKDIR) $(DEPS_DIR)
	$(MKDIR) $(BIN_DIR)

# Link the main executable
$(MAIN_TARGET): $(MAIN_OBJECT) | dirs
	@echo "Linking $(MAIN_TARGET)..."
	$(CXX) $(MAIN_OBJECT) -o $@ $(LDFLAGS) $(SDL3_LIBS) $(DL_LIBS)
	@echo "Main executable complete: $(MAIN_TARGET)"

# Build the game library
$(GAME_TARGET): $(GAME_OBJECT) | dirs
	@echo "Linking $(GAME_TARGET)..."
	$(CXX) $(SHARED_FLAGS) $(GAME_OBJECT) -o $@ $(LDFLAGS)
	@echo "Game library complete: $(GAME_TARGET)"

# Compile main.cpp
$(MAIN_OBJECT): $(MAIN_SOURCE) | dirs
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -MF $(MAIN_DEPENDS) -c $< -o $@

# Compile game.cpp
$(GAME_OBJECT): $(GAME_SOURCE) | dirs
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(SHARED_FLAGS) $(INCLUDES) -MMD -MP -MF $(GAME_DEPENDS) -c $< -o $@

# Install target - copy binaries and assets to dist/
install: all
	@echo "Installing to $(DIST_DIR)..."
	$(MKDIR) $(DIST_DIR)
	$(COPY) $(MAIN_TARGET) $(DIST_DIR)$(PATH_SEP)
	$(COPY) $(GAME_TARGET) $(DIST_DIR)$(PATH_SEP)
ifeq ($(wildcard $(ASSETS_DIR)),$(ASSETS_DIR))
	$(COPYR) $(ASSETS_DIR) $(DIST_DIR)$(PATH_SEP)$(ASSETS_DIR)
else
	@echo "Warning: $(ASSETS_DIR) directory not found, skipping asset copy"
endif
	@echo "Installation complete in $(DIST_DIR)/"

# Include dependency files
-include $(MAIN_DEPENDS) $(GAME_DEPENDS)

# Clean build files
clean:
	@echo "Cleaning build files..."
ifeq ($(PLATFORM),Windows)
	-$(RM) $(BUILD_DIR)\*.* /S 2>nul || true
	-rmdir $(BUILD_DIR) /S /Q 2>nul || true
	-$(RM) $(DIST_DIR)\*.* /S 2>nul || true
	-rmdir $(DIST_DIR) /S /Q 2>nul || true
else
	$(RM) -r $(BUILD_DIR) $(DIST_DIR)
endif
	@echo "Clean complete."

# Help target
help:
	@echo "Available targets:"
	@echo "  all      - Build the project (main executable + game library)"
	@echo "  debug    - Build in debug mode"
	@echo "  release  - Build in release mode"
	@echo "  install  - Build and copy binaries + assets to $(DIST_DIR)/"
	@echo "  clean    - Remove all build files and dist directory"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Build structure:"
	@echo "  $(BUILD_DIR)/$(BUILD_SUFFIX)/obj/     - Object files (.o)"
	@echo "  $(BUILD_DIR)/$(BUILD_SUFFIX)/deps/    - Dependency files (.d)"
	@echo "  $(BUILD_DIR)/$(BUILD_SUFFIX)/bin/     - Built binaries"
	@echo "  $(DIST_DIR)/                          - Distribution files (install target)"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Debug build"
	@echo "  make release install    # Release build and install"
	@echo "  make MODE=release       # Release build (alternative)"
	@echo "  make install            # Debug build and install"
