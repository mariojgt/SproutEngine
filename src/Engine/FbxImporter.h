#pragma once
#include "Model.h"
#include <optional>
#include <string>

// Loads a model file (FBX/OBJ etc.) and returns parsed geometry and materials.
// Returns std::nullopt on failure.
std::optional<Model> LoadModel(const std::string &path);
