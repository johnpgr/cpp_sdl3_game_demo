# Project configuration
PROJECT_NAME := game
SRC_DIR := src
BUILD_DIR := build

# Compiler settings
CXX := clang++
CXXFLAGS := -std=c++23 -Wall -Wextra -O2
DEBUG_FLAGS := -g -DDEBUG -O0
RELEASE_FLAGS := -DNDEBUG -O3

# Platform detection
ifeq ($(OS),Windows_NT)
    PLATFORM := Windows
    EXECUTABLE := $(PROJECT_NAME).exe
    RM := del /Q
    MKDIR := mkdir
    PATH_SEP := \\
    # Windows-specific flags
    LDFLAGS += -static-libgcc -static-libstdc++
else
    PLATFORM := Unix
    EXECUTABLE := $(PROJECT_NAME)
    RM := rm -f
    MKDIR := mkdir -p
    PATH_SEP := /
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        PLATFORM := macOS
    endif
endif

# SDL3 configuration
ifeq ($(PLATFORM),Windows)
    # Windows: Assume SDL3 is installed in a standard location or set SDL3_DIR
    ifdef SDL3_DIR
        SDL3_INCLUDE := -I$(SDL3_DIR)/include
        SDL3_LIBS := -L$(SDL3_DIR)/lib -lSDL3 -lSDL3main
    else
        # Default Windows paths - adjust as needed
        SDL3_INCLUDE := -IC:/SDL3/include
        SDL3_LIBS := -LC:/SDL3/lib -lSDL3 -lSDL3main
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
        SDL3_LIBS := -lSDL3 -lSDL3main
    endif
    
    # macOS specific
    ifeq ($(PLATFORM),macOS)
        SDL3_LIBS += -framework Cocoa -framework IOKit -framework CoreVideo
    endif
endif

# Include directories
INCLUDES := -I$(SRC_DIR) $(SDL3_INCLUDE)

# Find all source files
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPENDS := $(OBJECTS:.o=.d)

# Build modes
MODE ?= debug
ifeq ($(MODE),debug)
    CXXFLAGS += $(DEBUG_FLAGS)
    BUILD_DIR := $(BUILD_DIR)/debug
else
    CXXFLAGS += $(RELEASE_FLAGS)
    BUILD_DIR := $(BUILD_DIR)/release
endif

# Final paths
TARGET := $(BUILD_DIR)/$(EXECUTABLE)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPENDS := $(OBJECTS:.o=.d)

# Phony targets
.PHONY: all clean debug release help

# Default target
all: $(TARGET)

# Debug build
debug:
	$(MAKE) MODE=debug

# Release build
release:
	$(MAKE) MODE=release

# Link the executable
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(SDL3_LIBS)
	@echo "Build complete: $(TARGET)"

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Create build directory
$(BUILD_DIR):
	$(MKDIR) $(BUILD_DIR)

# Include dependency files
-include $(DEPENDS)

# Clean build files
clean:
	@echo "Cleaning build files..."
ifeq ($(PLATFORM),Windows)
	-$(RM) $(BUILD_DIR)\*.o $(BUILD_DIR)\*.d $(BUILD_DIR)\*.exe 2>nul || true
	-rmdir $(BUILD_DIR) 2>nul || true
else
	$(RM) -r build
endif
	@echo "Clean complete."

# Help target
help:
	@echo "Available targets:"
	@echo "  all      - Build the project (default: debug mode)"
	@echo "  debug    - Build in debug mode"
	@echo "  release  - Build in release mode"
	@echo "  clean    - Remove all build files"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Debug build"
	@echo "  make debug              # Debug build (alternative)"
	@echo "  make release            # Release build"
	@echo "  make MODE=release       # Release build (alternative)"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Release build"
	@echo "  make debug              # Debug build"
	@echo "  make MODE=debug         # Debug build (alternative)"
