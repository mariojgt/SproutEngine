cmake_minimum_required(VERSION 3.22)
project(SproutEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Put binaries in a predictable place
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find dependencies (recommended via vcpkg manifest mode)
#   vcpkg will provide CONFIG packages for SDL2/bgfx.
find_package(SDL2 CONFIG REQUIRED)
find_package(bgfx CONFIG REQUIRED)

# Engine library
add_library(SproutEngine STATIC
    engine/Core/Application.cpp
    engine/Core/Application.h
    engine/Platform/Window.cpp
    engine/Platform/Window.h
    engine/Render/Renderer.cpp
    engine/Render/Renderer.h
)

target_include_directories(SproutEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
)

target_link_libraries(SproutEngine PUBLIC
    SDL2::SDL2
    bgfx::bgfx
    bgfx::bx
    bgfx::bimg
)

# On Windows, also link SDL2main for the entry point
if (WIN32)
  target_link_libraries(SproutEngine PUBLIC SDL2::SDL2main)
endif()

# On Apple, link common frameworks often required by bgfx/SDL
if (APPLE)
  target_link_libraries(SproutEngine PUBLIC "-framework Cocoa" "-framework QuartzCore" "-framework Metal")
endif()

# Sample app
add_executable(SproutDemo samples/Triangle/main.cpp)
target_link_libraries(SproutDemo PRIVATE SproutEngine)

# Enable warnings
if (MSVC)
  target_compile_options(SproutEngine PRIVATE /W4 /permissive-)
  target_compile_options(SproutDemo PRIVATE /W4 /permissive-)
else()
  target_compile_options(SproutEngine PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(SproutDemo PRIVATE -Wall -Wextra -Wpedantic)
endif()
