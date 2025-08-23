#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <entt/entt.hpp>

/**
 * Enhanced Transform component with hierarchy support
 */
struct Transform {
    glm::vec3 position{0.0f};
    glm::vec3 rotationEuler{0.0f}; // degrees
    glm::vec3 scale{1.0f};
    
    // Hierarchy support
    entt::entity parent{entt::null};
    
    // Helper methods
    glm::mat4 GetLocalMatrix() const {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(rotationEuler.x), glm::vec3(1, 0, 0));
        R = glm::rotate(R, glm::radians(rotationEuler.y), glm::vec3(0, 1, 0));
        R = glm::rotate(R, glm::radians(rotationEuler.z), glm::vec3(0, 0, 1));
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
        return T * R * S;
    }
    
    glm::quat GetRotationQuaternion() const {
        return glm::quat(glm::radians(rotationEuler));
    }
    
    void SetRotationQuaternion(const glm::quat& quat) {
        rotationEuler = glm::degrees(glm::eulerAngles(quat));
    }
    
    glm::vec3 GetForward() const {
        glm::quat quat = GetRotationQuaternion();
        return quat * glm::vec3(0, 0, 1);
    }
    
    glm::vec3 GetRight() const {
        glm::quat quat = GetRotationQuaternion();
        return quat * glm::vec3(1, 0, 0);
    }
    
    glm::vec3 GetUp() const {
        glm::quat quat = GetRotationQuaternion();
        return quat * glm::vec3(0, 1, 0);
    }
};
