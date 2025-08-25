\
find_program(SHADERC_EXECUTABLE NAMES shaderc
    HINTS
        ${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/tools/bgfx
        ${VCPKG_ROOT}/packages/bgfx_*/*/tools/bgfx
        ${VCPKG_ROOT}/packages/bgfx_*/*/tools
        ${VCPKG_ROOT}/installed/*/tools/bgfx
        ${CMAKE_SOURCE_DIR}/vcpkg/installed/*/tools/bgfx
)
if(NOT SHADERC_EXECUTABLE)
    message(WARNING "bgfx shaderc not found. Shaders will not be compiled.")
    set(SPROUT_SHADERS_COMPILED FALSE PARENT_SCOPE)
    return()
endif()

if(WIN32)
    set(SPROUT_BACKEND d3d11)
    set(SPROUT_VS_PROFILE vs_5_0)
    set(SPROUT_FS_PROFILE ps_5_0)
    set(SPROUT_PLATFORM windows)
elseif(APPLE)
    set(SPROUT_BACKEND metal)
    set(SPROUT_VS_PROFILE metal)
    set(SPROUT_FS_PROFILE metal)
    set(SPROUT_PLATFORM osx)
else()
    set(SPROUT_BACKEND glsl)
    set(SPROUT_VS_PROFILE 430)
    set(SPROUT_FS_PROFILE 430)
    set(SPROUT_PLATFORM linux)
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders/${SPROUT_BACKEND})

set(SPROUT_SHADER_SOURCES
    ${CMAKE_SOURCE_DIR}/shaders/vs_lit.sc
    ${CMAKE_SOURCE_DIR}/shaders/fs_lit.sc
)

set(SPROUT_SHADER_OUTPUTS
    ${CMAKE_BINARY_DIR}/shaders/${SPROUT_BACKEND}/vs_lit.bin
    ${CMAKE_BINARY_DIR}/shaders/${SPROUT_BACKEND}/fs_lit.bin
)

add_custom_command(
    OUTPUT ${SPROUT_SHADER_OUTPUTS}
    COMMAND ${SHADERC_EXECUTABLE} -f ${CMAKE_SOURCE_DIR}/shaders/vs_lit.sc -o ${CMAKE_BINARY_DIR}/shaders/${SPROUT_BACKEND}/vs_lit.bin --type v --platform ${SPROUT_PLATFORM} --profile ${SPROUT_VS_PROFILE} -i ${CMAKE_SOURCE_DIR}/shaders
    COMMAND ${SHADERC_EXECUTABLE} -f ${CMAKE_SOURCE_DIR}/shaders/fs_lit.sc -o ${CMAKE_BINARY_DIR}/shaders/${SPROUT_BACKEND}/fs_lit.bin --type f --platform ${SPROUT_PLATFORM} --profile ${SPROUT_FS_PROFILE} -i ${CMAKE_SOURCE_DIR}/shaders
    DEPENDS ${SPROUT_SHADER_SOURCES}
    COMMENT "Compiling Sprout lit shaders for ${SPROUT_BACKEND}"
    VERBATIM
)

add_custom_target(SproutShaders ALL DEPENDS ${SPROUT_SHADER_OUTPUTS})
set(SPROUT_SHADERS_COMPILED TRUE PARENT_SCOPE)
