#pragma once
#include <string>
#include <optional>
#include <vector>
#include <cstdint>

double GetFileWriteTime(const std::string& path);
std::optional<std::string> ReadTextFile(const std::string& path);
std::vector<uint8_t> ReadBinaryFile(const std::string& path);
