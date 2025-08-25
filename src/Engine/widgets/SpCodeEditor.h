#pragma once
#ifdef SP_TOOLCHAIN_ENABLED
#include <string>

class SpCodeEditor {
public:
    void open(const std::string& path);
    void draw();
private:
    bool m_open = false;
    std::string m_path;
    std::string m_buffer;
};
#endif // SP_TOOLCHAIN_ENABLED
