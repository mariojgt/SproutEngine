#include "GltfLoader.h"
#include <tiny_gltf.h>
#include <iostream>
#include <cassert>

using namespace Sprout::Assets;

static const float* get_floats(const tinygltf::Model& model, const tinygltf::Accessor& accessor)
{
    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buf = model.buffers[view.buffer];
    return reinterpret_cast<const float*>(&buf.data[accessor.byteOffset + view.byteOffset]);
}

static const void* get_indices_ptr(const tinygltf::Model& model, const tinygltf::Accessor& accessor)
{
    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buf = model.buffers[view.buffer];
    return reinterpret_cast<const void*>(&buf.data[accessor.byteOffset + view.byteOffset]);
}

std::optional<LoadedMesh> Sprout::Assets::LoadFirstMeshFromGLTF(const char* path)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    tinygltf::Error error;
    tinygltf::Warning warning;

    bool ok = loader.LoadASCIIFromFile(&model, &error, &warning, path);
    if (!ok) {
        ok = loader.LoadBinaryFromFile(&model, &error, &warning, path);
    }
    if (!warning.empty()) { std::cerr << "[tinygltf] warning: " << warning << "\\n"; }
    if (!ok) {
        std::cerr << "[tinygltf] error: " << error << "\\n";
        return std::nullopt;
    }

    if (model.meshes.empty() || model.meshes[0].primitives.empty()) {
        std::cerr << "[tinygltf] no meshes/primitives in file\\n";
        return std::nullopt;
    }

    const tinygltf::Primitive& prim = model.meshes[0].primitives[0];
    if (prim.mode != TINYGLTF_MODE_TRIANGLES) {
        std::cerr << "[tinygltf] first primitive not triangles\\n";
        return std::nullopt;
    }

    LoadedMesh out;

    // Positions
    auto itPos = prim.attributes.find("POSITION");
    if (itPos == prim.attributes.end()) {
        std::cerr << "[tinygltf] no POSITION\\n";
        return std::nullopt;
    }
    const tinygltf::Accessor& posAcc = model.accessors[itPos->second];
    const float* pos = get_floats(model, posAcc);
    out.positions.resize(posAcc.count);
    for (size_t i = 0; i < posAcc.count; ++i) {
        out.positions[i] = { pos[i*3+0], pos[i*3+1], pos[i*3+2] };
    }

    // Normals (optional)
    auto itNrm = prim.attributes.find("NORMAL");
    if (itNrm != prim.attributes.end()) {
        const tinygltf::Accessor& nrmAcc = model.accessors[itNrm->second];
        const float* nrm = get_floats(model, nrmAcc);
        out.normals.resize(nrmAcc.count);
        for (size_t i = 0; i < nrmAcc.count; ++i) {
            out.normals[i] = { nrm[i*3+0], nrm[i*3+1], nrm[i*3+2] };
        }
    }

    // UVs (optional, use TEXCOORD_0)
    auto itUv = prim.attributes.find("TEXCOORD_0");
    if (itUv != prim.attributes.end()) {
        const tinygltf::Accessor& uvAcc = model.accessors[itUv->second];
        const float* uv = get_floats(model, uvAcc);
        out.uvs.resize(uvAcc.count);
        for (size_t i = 0; i < uvAcc.count; ++i) {
            out.uvs[i] = { uv[i*2+0], uv[i*2+1] };
        }
    }

    // Indices
    if (prim.indices < 0) {
        // no indices; generate a trivial sequential index buffer
        out.indices.resize(out.positions.size());
        for (uint32_t i = 0; i < out.indices.size(); ++i) out.indices[i] = i;
    } else {
        const tinygltf::Accessor& idxAcc = model.accessors[prim.indices];
        const void* base = get_indices_ptr(model, idxAcc);
        out.indices.resize(idxAcc.count);

        switch (idxAcc.componentType) {
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                const uint8_t* p = static_cast<const uint8_t*>(base);
                for (size_t i = 0; i < idxAcc.count; ++i) out.indices[i] = p[i];
            } break;
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                const uint16_t* p = static_cast<const uint16_t*>(base);
                for (size_t i = 0; i < idxAcc.count; ++i) out.indices[i] = p[i];
            } break;
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                const uint32_t* p = static_cast<const uint32_t*>(base);
                for (size_t i = 0; i < idxAcc.count; ++i) out.indices[i] = p[i];
            } break;
            default:
                std::cerr << "[tinygltf] unsupported index type\\n";
                return std::nullopt;
        }
    }

    return out;
}
