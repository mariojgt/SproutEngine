#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <optional>
#include <string>

namespace Sprout::Assets
{
    struct LoadedMesh {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<uint32_t>  indices;
    };

    // Loads the first mesh primitive from a glTF file (triangles only).
    std::optional<LoadedMesh> LoadFirstMeshFromGLTF(const char* path);
}
