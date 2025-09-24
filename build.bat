@echo off
setlocal enabledelayedexpansion

REM Project configuration
set PROJECT_NAME=game
set SRC_DIR=src
set BUILD_DIR=build
set ASSETS_DIR=assets
set SHADER_SOURCE_DIR=%ASSETS_DIR%\shaders\source
set SHADER_OUT_DIR=%ASSETS_DIR%\shaders\compiled

REM Compiler settings
set CXX=clang++
set CXXFLAGS=-std=c++23 -Wall -Wextra -Wno-address-of-temporary -Wno-missing-designated-field-initializers
set DEBUG_FLAGS=-g -DDEBUG
set RELEASE_FLAGS=-DNDEBUG -O3
set LDFLAGS=-fuse-ld=lld -Wl,/SUBSYSTEM:WINDOWS -Wl,/NOIMPLIB
set SHARED_FLAGS=-shared

REM Build mode from environment variable, default to debug
if "%BUILD_MODE%"=="" set BUILD_MODE=debug

REM Platform specific settings
set LIBRARY_EXT=.dll
set DL_LIBS=-ldbghelp

REM SDL configuration - check for SDL3_DIR environment variable
if "%SDL3_DIR%"=="" (
    echo Warning: SDL3_DIR not set, using default paths
    set SDL3_INCLUDE=-IC:\SDKs\SDL3\include
    set SDL3_LIBS=-LC:\SDKs\SDL3\lib -lSDL3
    set SDL3_IMAGE_INCLUDE=-IC:\SDKs\SDL3_image\include
    set SDL3_IMAGE_LIBS=-LC:\SDKs\SDL3_image\lib -lSDL3_image
    set SDL3_TTF_INCLUDE=-IC:\SDKs\SDL3_ttf\include
    set SDL3_TTF_LIBS=-LC:\SDKs\SDL3_ttf\lib -lSDL3_ttf
) else (
    set SDL3_INCLUDE=-I%SDL3_DIR%\include
    set SDL3_LIBS=-L%SDL3_DIR%\lib -lSDL3
    set SDL3_IMAGE_INCLUDE=-I%SDL3_DIR%\include
    set SDL3_IMAGE_LIBS=-L%SDL3_DIR%\lib -lSDL3_image
    set SDL3_TTF_INCLUDE=-I%SDL3_DIR%\include
    set SDL3_TTF_LIBS=-L%SDL3_DIR%\lib -lSDL3_ttf
)

set SDL_INCLUDE=%SDL3_INCLUDE% %SDL3_IMAGE_INCLUDE% %SDL3_TTF_INCLUDE%
set SDL_LIBS=%SDL3_LIBS% %SDL3_IMAGE_LIBS% %SDL3_TTF_LIBS%

REM Build mode setup
if /i "%BUILD_MODE%"=="release" (
    set CXXFLAGS=%CXXFLAGS% %RELEASE_FLAGS%
    set BUILD_SUFFIX=release
) else (
    set CXXFLAGS=%CXXFLAGS% %DEBUG_FLAGS%
    set BUILD_SUFFIX=debug
)

set OBJ_DIR=%BUILD_DIR%\%BUILD_SUFFIX%\obj
set BIN_DIR=%BUILD_DIR%\%BUILD_SUFFIX%\bin
set EXECUTABLE=%BIN_DIR%\%PROJECT_NAME%.exe
set GAME_LIBRARY=%BIN_DIR%\libgame%LIBRARY_EXT%
set MAIN_PDB=%BIN_DIR%\%PROJECT_NAME%.pdb
set GAME_PDB=%BIN_DIR%\libgame.pdb

echo Building in %BUILD_SUFFIX% mode...

REM Create directories
if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
if not exist "%SHADER_OUT_DIR%" mkdir "%SHADER_OUT_DIR%"

REM Compile shaders if shadercross is available
where shadercross >nul 2>nul
if %errorlevel% equ 0 (
    if exist "%SHADER_SOURCE_DIR%" (
        echo Compiling shaders...
        
        REM Compile vertex shaders
        for %%f in ("%SHADER_SOURCE_DIR%\*.vert.hlsl") do (
            if exist "%%f" (
                echo   Compiling vertex shader: %%~nf
                shadercross "%%f" -o "%SHADER_OUT_DIR%\%%~nf.spv" 2>nul || echo     Warning: Failed to compile %%~nf.spv
                shadercross "%%f" -o "%SHADER_OUT_DIR%\%%~nf.msl" 2>nul || echo     Warning: Failed to compile %%~nf.msl
                shadercross "%%f" -o "%SHADER_OUT_DIR%\%%~nf.dxil" 2>nul || echo     Warning: Failed to compile %%~nf.dxil
            )
        )
        
        REM Compile fragment shaders
        for %%f in ("%SHADER_SOURCE_DIR%\*.frag.hlsl") do (
            if exist "%%f" (
                echo   Compiling fragment shader: %%~nf
                shadercross "%%f" -o "%SHADER_OUT_DIR%\%%~nf.spv" 2>nul || echo     Warning: Failed to compile %%~nf.spv
                shadercross "%%f" -o "%SHADER_OUT_DIR%\%%~nf.msl" 2>nul || echo     Warning: Failed to compile %%~nf.msl
                shadercross "%%f" -o "%SHADER_OUT_DIR%\%%~nf.dxil" 2>nul || echo     Warning: Failed to compile %%~nf.dxil
            )
        )
        
        REM Compile compute shaders
        for %%f in ("%SHADER_SOURCE_DIR%\*.comp.hlsl") do (
            if exist "%%f" (
                echo   Compiling compute shader: %%~nf
                shadercross "%%f" -o "%SHADER_OUT_DIR%\%%~nf.spv" 2>nul || echo     Warning: Failed to compile %%~nf.spv
                shadercross "%%f" -o "%SHADER_OUT_DIR%\%%~nf.msl" 2>nul || echo     Warning: Failed to compile %%~nf.msl
                shadercross "%%f" -o "%SHADER_OUT_DIR%\%%~nf.dxil" 2>nul || echo     Warning: Failed to compile %%~nf.dxil
            )
        )
    ) else (
        echo Shader source directory not found: %SHADER_SOURCE_DIR%
    )
) else (
    echo shadercross not found, skipping shader compilation
)

REM Copy assets
if exist "%ASSETS_DIR%" (
    echo Copying assets...
    xcopy /E /I /Y "%ASSETS_DIR%" "%BIN_DIR%\%ASSETS_DIR%\" >nul
) else (
    echo Warning: %ASSETS_DIR% directory not found, skipping asset copy
)

REM Compile source files
set INCLUDES=-I%SRC_DIR% %SDL_INCLUDE%

echo Compiling sources...
%CXX% %CXXFLAGS% %INCLUDES% -c "%SRC_DIR%\main.cpp" -o "%OBJ_DIR%\main.o"
if %errorlevel% neq 0 (
    echo Error: Failed to compile main.cpp
    exit /b 1
)

%CXX% %CXXFLAGS% %INCLUDES% -c "%SRC_DIR%\game.cpp" -o "%OBJ_DIR%\game.o"
if %errorlevel% neq 0 (
    echo Error: Failed to compile game.cpp
    exit /b 1
)

REM Link executables
echo Linking targets...

REM Try to link main executable, but don't fail the script if it's locked
%CXX% %CXXFLAGS% "%OBJ_DIR%\main.o" -o "%EXECUTABLE%" %LDFLAGS% -Wl,/PDB:%MAIN_PDB% %SDL_LIBS% %DL_LIBS% >nul 2>nul
if %errorlevel% equ 0 (
    echo Main executable: %EXECUTABLE%
) else (
    echo Warning: Failed to link main executable ^(file may be locked by debugger^)
)

REM Always try to build the game library
%CXX% %CXXFLAGS% %SHARED_FLAGS% "%OBJ_DIR%\game.o" -o "%GAME_LIBRARY%" %LDFLAGS% -Wl,/PDB:%GAME_PDB% %SDL_LIBS% %DL_LIBS%
if %errorlevel% neq 0 (
    echo Error: Failed to link game library
    exit /b 1
)

echo Build complete!
echo Game library: %GAME_LIBRARY%
