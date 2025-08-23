#include "FileUtil.h"
#include <fstream>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

double GetFileWriteTime(const std::string& path) {
    try {
        auto ftime = fs::last_write_time(path);
        auto sctp = decltype(ftime)::clock::to_sys(ftime);
        return std::chrono::duration<double>(sctp.time_since_epoch()).count();
    } catch(...) { return 0.0; }
}

std::optional<std::string> ReadTextFile(const std::string& path){
    std::ifstream ifs(path, std::ios::in);
    if(!ifs) return std::nullopt;
    std::string s((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return s;
}

std::vector<uint8_t> ReadBinaryFile(const std::string& path){
    std::ifstream ifs(path, std::ios::binary);
    if(!ifs) return {};
    return {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
}
