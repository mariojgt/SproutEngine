#include "Renderer.h"
#define GLFW_INCLUDE_NONE
#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#ifndef SE_ASSETS_DIR
#define SE_ASSETS_DIR "assets"
#endif

static std::string LoadText(const std::string& path){
    std::ifstream ifs(path);
    std::stringstream ss; ss << ifs.rdbuf();
    return ss.str();
}

bool Renderer::init(GLFWwindow* window){
    (void)window;
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cerr << "Failed to load GL" << std::endl; return false;
    }
    glEnable(GL_DEPTH_TEST);

    // Shaders
    if(!loadShaders(std::string(SE_ASSETS_DIR) + "/shaders/basic.vert",
                    std::string(SE_ASSETS_DIR) + "/shaders/basic.frag")) return false;

    // Unit cube (pos, normal)
    float v[] = {
      // positions         // normals
      -0.5f,-0.5f,-0.5f,   0,-1,0,
       0.5f,-0.5f,-0.5f,   0,-1,0,
       0.5f,-0.5f, 0.5f,   0,-1,0,
      -0.5f,-0.5f, 0.5f,   0,-1,0,
      -0.5f, 0.5f,-0.5f,   0, 1,0,
       0.5f, 0.5f,-0.5f,   0, 1,0,
       0.5f, 0.5f, 0.5f,   0, 1,0,
      -0.5f, 0.5f, 0.5f,   0, 1,0,
    };
    unsigned idx[] = {
      0,1,2, 2,3,0, // bottom
      4,5,6, 6,7,4, // top
      0,1,5, 5,4,0, // front
      2,3,7, 7,6,2, // back
      1,2,6, 6,5,1, // right
      3,0,4, 4,7,3  // left
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return true;
}

void Renderer::shutdown(){
    if(m_program) glDeleteProgram(m_program);
    if(m_vbo) glDeleteBuffers(1,&m_vbo);
    if(m_ebo) glDeleteBuffers(1,&m_ebo);
    if(m_vao) glDeleteVertexArrays(1,&m_vao);
}

void Renderer::beginFrame(int w, int h){
    glViewport(0,0,w,h);
    glClearColor(0.08f,0.09f,0.11f,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void Renderer::drawCube(const glm::mat4& mvp){
    glUseProgram(m_program);
    glUniformMatrix4fv(m_uMVP, 1, GL_FALSE, &mvp[0][0]);
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::drawCube(const glm::mat4& mvp, const glm::vec3& tint) {
    glUseProgram(m_program);
    glUniformMatrix4fv(m_uMVP, 1, GL_FALSE, &mvp[0][0]);
    int loc = glGetUniformLocation(m_program, "uTint");
    if (loc >= 0) glUniform3f(loc, tint.x, tint.y, tint.z);
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    // reset tint to white to avoid leaking to other draws
    if (loc >= 0) glUniform3f(loc, 1.0f, 1.0f, 1.0f);
}

void Renderer::endFrame(){
    // nothing for now
}

unsigned Renderer::compileShader(unsigned type, const std::string& src){
    unsigned s = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(s,1,&c,nullptr);
    glCompileShader(s);
    int ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if(!ok){ char log[1024]; glGetShaderInfoLog(s,1024,nullptr,log); std::cerr<<"Shader error: "<<log<<"\n"; }
    return s;
}

unsigned Renderer::linkProgram(unsigned vs, unsigned fs){
    unsigned p = glCreateProgram();
    glAttachShader(p, vs); glAttachShader(p, fs);
    glLinkProgram(p);
    int ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if(!ok){ char log[1024]; glGetProgramInfoLog(p,1024,nullptr,log); std::cerr<<"Link error: "<<log<<"\n"; }
    glDeleteShader(vs); glDeleteShader(fs);
    return p;
}

bool Renderer::loadShaders(const std::string& vp, const std::string& fp){
    auto vsrc = LoadText(vp);
    auto fsrc = LoadText(fp);
    unsigned vs = compileShader(GL_VERTEX_SHADER, vsrc);
    unsigned fs = compileShader(GL_FRAGMENT_SHADER, fsrc);
    m_program = linkProgram(vs, fs);
    m_uMVP = glGetUniformLocation(m_program, "uMVP");
    // Ensure tint uniform exists and initialize to white
    int tintLoc = glGetUniformLocation(m_program, "uTint");
    if (tintLoc >= 0) {
        glUseProgram(m_program);
        glUniform3f(tintLoc, 1.0f, 1.0f, 1.0f);
        glUseProgram(0);
    }
    return m_program != 0;
}
