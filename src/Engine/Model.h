#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex {
  glm::vec3 position{0.0f};
  glm::vec3 normal{0.0f};
  glm::vec2 texCoord{0.0f};
};

struct Material {
  std::string name;
  glm::vec3 diffuseColor{1.0f};
  std::string diffuseTexture; // relative path to diffuse texture
};

struct Mesh {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  Material material;
};

struct Model {
  std::vector<Mesh> meshes;
};
