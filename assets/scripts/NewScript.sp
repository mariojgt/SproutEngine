// NewScript.sp - SproutScript Template
// This is a template for creating new SproutScript behaviors

class NewScript : public ScriptComponent {
private:
    // Add your private variables here
    float exampleFloat = 1.0f;
    int exampleInt = 0;
    bool exampleBool = false;

public:
    // Called when the entity is first created
    void OnStart(EntityID id) override {
        Print("NewScript OnStart for entity " + std::to_string(id));
        // Initialize your script here
    }

    // Called every frame
    void OnTick(EntityID id, float deltaTime) override {
        // Update logic goes here
        // Example: Move, rotate, or modify the entity

        // Get current transform
        Vector3 position = GetPosition(id);
        Vector3 rotation = GetRotation(id);
        Vector3 scale = GetScale(id);

        // Modify as needed
        // SetPosition(id, position);
        // SetRotation(id, rotation);
        // SetScale(id, scale);
    }

    // Called when another entity collides with this one
    void OnCollision(EntityID id, EntityID other) override {
        Print("Collision detected between " + std::to_string(id) + " and " + std::to_string(other));
    }

    // Called when the entity is destroyed
    void OnDestroy(EntityID id) override {
        Print("NewScript OnDestroy for entity " + std::to_string(id));
        // Cleanup code here
    }

    // Inspector GUI - appears in the Details panel
    void OnInspector() override {
        ImGui::Text("Script Properties:");
        ImGui::SliderFloat("Example Float", &exampleFloat, 0.0f, 10.0f);
        ImGui::SliderInt("Example Int", &exampleInt, 0, 100);
        ImGui::Checkbox("Example Bool", &exampleBool);

        ImGui::Separator();
        if (ImGui::Button("Reset Values")) {
            exampleFloat = 1.0f;
            exampleInt = 0;
            exampleBool = false;
        }
    }
};

// Register the script with the engine
REGISTER_SCRIPT(NewScript);
