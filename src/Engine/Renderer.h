#pragma once
#include <glm/glm.hpp>
#include <string>

struct GLFWwindow;

class Renderer {
public:
    bool init(GLFWwindow* window);
    void shutdown();

    void beginFrame(int width, int height);
    void drawCube(const glm::mat4& mvp);
    // tint multiplies the base color (use to highlight selected objects)
    void drawCube(const glm::mat4& mvp, const glm::vec3& tint);
    void endFrame();

private:
    unsigned int m_program = 0;
    unsigned int m_vao = 0, m_vbo = 0, m_ebo = 0;
    int m_uMVP = -1;

    unsigned int compileShader(unsigned int type, const std::string& src);
    unsigned int linkProgram(unsigned int vs, unsigned int fs);
    bool loadShaders(const std::string& vertPath, const std::string& fragPath);
};
