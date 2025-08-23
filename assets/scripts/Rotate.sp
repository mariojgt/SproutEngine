// Rotate.sp - SproutScript version of rotation behavior
// Converted from Rotate.lua for live editing in SproutEngine

class RotateScript : public ScriptComponent {
private:
    float speed = 45.0f; // degrees per second

public:
    void OnStart(EntityID id) override {
        Print("Rotate.sp OnStart for entity " + std::to_string(id));
    }

    void OnTick(EntityID id, float dt) override {
        Vector3 rotation = GetRotation(id);
        rotation.y += speed * dt;
        SetRotation(id, rotation);
    }

    // Inspector properties
    void OnInspector() override {
        ImGui::SliderFloat("Rotation Speed", &speed, 0.0f, 180.0f);
    }
};

// Register the script
REGISTER_SCRIPT(RotateScript);
