cmake_minimum_required(VERSION 3.22)
project(SproutEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Use vcpkg manifest mode; pass - DCMAKE_TOOLCHAIN_FILE = ... when configuring

find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(EnTT REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(Lua REQUIRED)
find_package(assimp CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h)

# Ensure all new files are included
set(ENGINE_SOURCES
    src/main.cpp
    src/Engine/Renderer.cpp
    src/Engine/Renderer.h
    src/Engine/Scene.cpp
    src/Engine/Scene.h
    src/Engine/Components.h
    src/Engine/Systems.cpp
    src/Engine/Systems.h
    src/Engine/Scripting.cpp
    src/Engine/Scripting.h
    src/Engine/Editor.cpp
    src/Engine/Editor.h
    src/Engine/HUD.cpp
    src/Engine/HUD.h
    src/Engine/FileUtil.cpp
    src/Engine/FileUtil.h
    src/Engine/Theme.cpp
    src/Engine/Theme.h
    src/Engine/VSGraph.cpp
    src/Engine/VSGraph.h
    src/Engine/FbxImporter.cpp
    src/Engine/FbxImporter.h
    src/Engine/Model.h
    src/Engine/UnrealEditorSimple.cpp
    src/Engine/UnrealEditorSimple.h
    src/Engine/ModernTheme.cpp
    src/Engine/ModernTheme.h
    # New Actor system files (temporarily disabled until compilation issues are resolved)
    # src/Engine/Actor.cpp
    # src/Engine/Actor.h
    # src/Engine/World.cpp
    # src/Engine/World.h
    # src/Engine/Transform.h
    # src/Engine/CoreComponents.cpp
    # src/Engine/CoreComponents.h
    # src/Engine/Blueprint.cpp
    # src/Engine/Blueprint.h
    # src/Engine/SproutScript.h
    # src/Engine/GameplayActors.cpp
    # src/Engine/GameplayActors.h
)

add_executable(SproutEngine ${ENGINE_SOURCES})

# Include dirs
target_include_directories(SproutEngine PRIVATE src external/imnodes_stub)

# Assets path macro
target_compile_definitions(SproutEngine PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD SE_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")

# Link libraries (handle different target names)
if(TARGET glad::glad)
  target_link_libraries(SproutEngine PRIVATE glad::glad)
else()
  target_link_libraries(SproutEngine PRIVATE glad)
endif()

if(TARGET glfw)
  target_link_libraries(SproutEngine PRIVATE glfw)
else()
  target_link_libraries(SproutEngine PRIVATE glfw3)
endif()

if(TARGET imgui::imgui)
  target_link_libraries(SproutEngine PRIVATE imgui::imgui)
else()
  target_link_libraries(SproutEngine PRIVATE imgui)
endif()

if(TARGET EnTT::EnTT)
  target_link_libraries(SproutEngine PRIVATE EnTT::EnTT)
endif()

if(TARGET sol2::sol2)
  target_link_libraries(SproutEngine PRIVATE sol2::sol2)
endif()

if(TARGET Lua::Lua)
  target_link_libraries(SproutEngine PRIVATE Lua::Lua)
else()
  target_include_directories(SproutEngine PRIVATE ${LUA_INCLUDE_DIR})
  target_link_libraries(SproutEngine PRIVATE ${LUA_LIBRARIES})
endif()

if(TARGET assimp::assimp)
  target_link_libraries(SproutEngine PRIVATE assimp::assimp)
else()
  target_link_libraries(SproutEngine PRIVATE assimp)
endif()

# Copy assets after build
add_custom_command(TARGET SproutEngine POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:SproutEngine>/assets")

# (backends removed; using TinyImGui)
if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
  set(VCPKG_INC "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
  if(EXISTS "${VCPKG_INC}/backends/imgui_impl_glfw.cpp")
    message(STATUS "Using ImGui backends from ${VCPKG_INC}/backends")
    target_sources(SproutEngine PRIVATE
      "${VCPKG_INC}/backends/imgui_impl_glfw.cpp"
      "${VCPKG_INC}/backends/imgui_impl_opengl3.cpp")
    target_include_directories(SproutEngine PRIVATE "${VCPKG_INC}")
  elseif(EXISTS "${VCPKG_INC}/imgui/backends/imgui_impl_glfw.cpp")
    message(STATUS "Using ImGui backends from ${VCPKG_INC}/imgui/backends")
    target_sources(SproutEngine PRIVATE
      "${VCPKG_INC}/imgui/backends/imgui_impl_glfw.cpp"
      "${VCPKG_INC}/imgui/backends/imgui_impl_opengl3.cpp")
    target_include_directories(SproutEngine PRIVATE "${VCPKG_INC}/imgui")
  else()
    message(WARNING "ImGui backend sources not found in vcpkg include dir. If build fails, install imgui dev headers with backends or vendor them.")
  endif()
endif()
