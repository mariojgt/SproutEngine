cmake_minimum_required(VERSION 3.22)
project(SproutEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages from vcpkg
find_package(SDL2 REQUIRED)
find_package(bgfx REQUIRED)
find_package(bimg REQUIRED)
find_package(bx REQUIRED)
find_package(EnTT REQUIRED)
find_package(glm REQUIRED)
find_package(Lua REQUIRED)
find_package(sol2 REQUIRED)
find_package(Stb REQUIRED)
find_package(tinygltf REQUIRED)

add_library(SproutEngine STATIC
    engine/Core/Application.cpp
    engine/Core/Application.h
    engine/Platform/Window.cpp
    engine/Platform/Window.h
    engine/Render/Renderer.cpp
    engine/Render/Renderer.h
    engine/Scene/Scene.cpp
    engine/Scene/Scene.h
    engine/ECS/Components.h
    engine/ECS/Systems.cpp
    engine/ECS/Systems.h
    engine/Assets/GltfLoader.cpp
    engine/Assets/GltfLoader.h
    engine/Math/Transform.h
    engine/Scripting/ScriptSystem.cpp
    engine/Scripting/ScriptSystem.h
)

target_include_directories(SproutEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
)

# Link all required libraries
target_link_libraries(SproutEngine PUBLIC
    SDL2::SDL2
    bgfx::bgfx
    bimg::bimg
    bx::bx
    EnTT::EnTT
    glm::glm
    ${LUA_LIBRARIES}
    sol2::sol2
    tinygltf::tinygltf
)

# Include directories for Lua
target_include_directories(SproutEngine PUBLIC ${LUA_INCLUDE_DIR})

# On Windows, also link SDL2main for the entry point
if (WIN32 AND TARGET SDL2::SDL2main)
  target_link_libraries(SproutEngine PUBLIC SDL2::SDL2main)
endif()

# On Apple, link common frameworks
if (APPLE)
  target_link_libraries(SproutEngine PUBLIC "-framework Cocoa" "-framework QuartzCore" "-framework Metal")
endif()

# Simple demo executable
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
