#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <cstdint>

namespace Sprout::ECS
{
    struct Transform {
        glm::vec3 position{0.0f, 0.0f, 0.0f};
        glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f}; // w, x, y, z
        glm::vec3 scale{1.0f, 1.0f, 1.0f};

        glm::mat4 localMatrix() const {
            glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
            glm::mat4 R = glm::mat4_cast(rotation);
            glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
            return T * R * S;
        }
    };

    struct Camera {
        float fovYDegrees = 60.0f;
        float nearPlane = 0.1f;
        float farPlane  = 100.0f;
        glm::mat4 view{1.0f};
        glm::mat4 proj{1.0f};
        bool primary = true;
    };

    struct Mesh {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<uint32_t>   indices;
        std::string debugName;
    };

    struct DirectionalLight {
        glm::vec3 direction{ -0.3f, -1.0f, -0.2f };
        glm::vec3 color{ 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;
    };
}
