#pragma once
#include <string>

namespace VSGraph {

enum class Premade {
  RotateOnTick,
  PrintHelloOnStart,
  PulseHealthBar
};

// Writes a Lua script under assets/scripts and returns the path
std::string Generate(const std::string& assetsDir, Premade type);

} // namespace VSGraph
