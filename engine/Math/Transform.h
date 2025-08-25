#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Sprout::Math
{
    inline glm::mat4 TRS(const glm::vec3& t, const glm::quat& r, const glm::vec3& s) {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), t);
        glm::mat4 R = glm::mat4_cast(r);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), s);
        return T * R * S;
    }
}
