#pragma once
#include <string>

namespace UI {

struct HUDState {
    float health{80.0f}; // 0..100
    float mana{50.0f};   // 0..100
    int score{1230};
    std::string title{"RPG Rush"};
};

class HUDRenderer {
public:
    void draw(const HUDState& s);
};

} // namespace UI
