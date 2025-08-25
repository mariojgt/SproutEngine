#include "FbxImporter.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

static Mesh ProcessMesh(const aiMesh *mesh, const aiScene *scene) {
  Mesh result;
  result.vertices.reserve(mesh->mNumVertices);
  for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
    Vertex v;
    v.position = {mesh->mVertices[i].x, mesh->mVertices[i].y,
                  mesh->mVertices[i].z};
    if (mesh->mNormals)
      v.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y,
                  mesh->mNormals[i].z};
    if (mesh->mTextureCoords[0])
      v.texCoord = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
    result.vertices.push_back(v);
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
    const aiFace &face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; ++j)
      result.indices.push_back(face.mIndices[j]);
  }

  if (mesh->mMaterialIndex >= 0 && scene->mMaterials) {
    aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];
    aiString name;
    if (AI_SUCCESS == mat->Get(AI_MATKEY_NAME, name))
      result.material.name = name.C_Str();

    aiColor3D color(1.0f, 1.0f, 1.0f);
    if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_DIFFUSE, color))
      result.material.diffuseColor = {color.r, color.g, color.b};

    aiString texPath;
    if (AI_SUCCESS == mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath))
      result.material.diffuseTexture = texPath.C_Str();
  }

  return result;
}

static void ProcessNode(Model &model, const aiNode *node,
                        const aiScene *scene) {
  for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
    const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    model.meshes.push_back(ProcessMesh(mesh, scene));
  }
  for (unsigned int i = 0; i < node->mNumChildren; ++i)
    ProcessNode(model, node->mChildren[i], scene);
}

std::optional<Model> LoadModel(const std::string &path) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_GenNormals |
                aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);
  if (!scene || !scene->mRootNode)
    return std::nullopt;

  Model model;
  ProcessNode(model, scene->mRootNode, scene);
  if (model.meshes.empty())
    return std::nullopt;
  return model;
}
