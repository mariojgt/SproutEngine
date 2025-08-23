// Blueprint Editor Implementation for SproutEngine
// This file contains the complete blueprint editor with .sp generation

#include "UnrealEditorSimple.h"
#include "Scripting.h"
#include "VSGraph.h"
#include <fstream>
#include <filesystem>

void UnrealEditor::DrawBlueprintGraph(entt::registry& registry, Scripting& scripting) {
    if (ImGui::Begin("Blueprint Graph & Code Editor")) {
        ImGui::Text("🔧 SproutEngine Blueprint Editor (.sp Script Generator)");
        ImGui::Separator();

        // Mode selector
        static bool visualMode = true;
        if (ImGui::RadioButton("Visual Blueprint", visualMode)) visualMode = true;
        ImGui::SameLine();
        if (ImGui::RadioButton("Code Editor", !visualMode)) visualMode = false;

        ImGui::Separator();

        if (visualMode) {
            // VISUAL BLUEPRINT EDITOR
            ImGui::Text("🎨 Visual Blueprint Editor");

            // Node Palette
            if (ImGui::CollapsingHeader("Node Palette", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Events:");
                ImGui::SameLine();
                if (ImGui::Button("OnStart")) {
                    BlueprintNode node;
                    node.id = nextNodeId++;
                    node.type = "Event";
                    node.name = "OnStart";
                    node.position = ImVec2(50, 50 + blueprintNodes.size() * 80);
                    node.outputPins = {node.id * 100 + 1}; // exec out
                    blueprintNodes.push_back(node);
                }
                ImGui::SameLine();
                if (ImGui::Button("OnTick")) {
                    BlueprintNode node;
                    node.id = nextNodeId++;
                    node.type = "Event";
                    node.name = "OnTick";
                    node.position = ImVec2(50, 50 + blueprintNodes.size() * 80);
                    node.outputPins = {node.id * 100 + 1}; // exec out
                    blueprintNodes.push_back(node);
                }

                ImGui::Text("Functions:");
                ImGui::SameLine();
                if (ImGui::Button("Print")) {
                    BlueprintNode node;
                    node.id = nextNodeId++;
                    node.type = "Function";
                    node.name = "Print";
                    node.position = ImVec2(250, 50 + blueprintNodes.size() * 80);
                    node.inputPins = {node.id * 100 + 1}; // exec in
                    node.outputPins = {node.id * 100 + 2}; // exec out
                    node.param1 = "Hello World!";
                    blueprintNodes.push_back(node);
                }
                ImGui::SameLine();
                if (ImGui::Button("SetRotation")) {
                    BlueprintNode node;
                    node.id = nextNodeId++;
                    node.type = "Function";
                    node.name = "SetRotation";
                    node.position = ImVec2(250, 50 + blueprintNodes.size() * 80);
                    node.inputPins = {node.id * 100 + 1, node.id * 100 + 3}; // exec in, vector in
                    node.outputPins = {node.id * 100 + 2}; // exec out
                    node.param1 = "0"; node.param2 = "90"; node.param3 = "0";
                    blueprintNodes.push_back(node);
                }

                ImGui::Text("Math:");
                ImGui::SameLine();
                if (ImGui::Button("Add")) {
                    BlueprintNode node;
                    node.id = nextNodeId++;
                    node.type = "Math";
                    node.name = "Add";
                    node.position = ImVec2(450, 50 + blueprintNodes.size() * 80);
                    node.inputPins = {node.id * 100 + 1, node.id * 100 + 2}; // A, B
                    node.outputPins = {node.id * 100 + 3}; // Result
                    node.param1 = "0"; node.param2 = "1";
                    blueprintNodes.push_back(node);
                }

                ImGui::Text("Variables:");
                ImGui::SameLine();
                if (ImGui::Button("Speed Variable")) {
                    BlueprintNode node;
                    node.id = nextNodeId++;
                    node.type = "Variable";
                    node.name = "speed";
                    node.position = ImVec2(450, 50 + blueprintNodes.size() * 80);
                    node.outputPins = {node.id * 100 + 1}; // value out
                    node.param1 = "90.0";
                    blueprintNodes.push_back(node);
                }
            }

            // Node Canvas (simplified visual representation)
            ImGui::Separator();
            ImGui::Text("Blueprint Canvas:");
            ImGui::BeginChild("NodeCanvas", ImVec2(0, 400), true, ImGuiWindowFlags_HorizontalScrollbar);

            // Draw nodes
            for (auto& node : blueprintNodes) {
                ImGui::PushID(node.id);

                // Node header with drag handle
                ImVec2 nodePos = node.position;
                ImGui::SetCursorPos(nodePos);

                // Node background
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 canvasPos = ImGui::GetCursorScreenPos();
                ImVec2 nodeSize(200, 100);

                ImU32 nodeColor = IM_COL32(60, 60, 60, 255);
                if (node.type == "Event") nodeColor = IM_COL32(200, 50, 50, 255);
                else if (node.type == "Function") nodeColor = IM_COL32(50, 100, 200, 255);
                else if (node.type == "Math") nodeColor = IM_COL32(50, 200, 50, 255);
                else if (node.type == "Variable") nodeColor = IM_COL32(200, 200, 50, 255);

                drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + nodeSize.x, canvasPos.y + nodeSize.y), nodeColor, 4.0f);
                drawList->AddRect(canvasPos, ImVec2(canvasPos.x + nodeSize.x, canvasPos.y + nodeSize.y), IM_COL32(255, 255, 255, 100), 4.0f, 0, 2.0f);

                // Node title
                ImGui::SetCursorPos(ImVec2(nodePos.x + 10, nodePos.y + 10));
                ImGui::Text("%s", node.name.c_str());

                // Node parameters
                if (!node.param1.empty()) {
                    ImGui::SetCursorPos(ImVec2(nodePos.x + 10, nodePos.y + 30));
                    char buf[128]; strncpy(buf, node.param1.c_str(), sizeof(buf));
                    ImGui::PushItemWidth(180);
                    if (ImGui::InputText("##param1", buf, sizeof(buf))) {
                        node.param1 = std::string(buf);
                    }
                    ImGui::PopItemWidth();
                }

                // Remove button
                ImGui::SetCursorPos(ImVec2(nodePos.x + 150, nodePos.y + 5));
                if (ImGui::SmallButton("X")) {
                    int removeId = node.id;
                    blueprintNodes.erase(std::remove_if(blueprintNodes.begin(), blueprintNodes.end(),
                        [removeId](const BlueprintNode& n) { return n.id == removeId; }), blueprintNodes.end());
                    // Also remove any connections
                    blueprintLinks.erase(std::remove_if(blueprintLinks.begin(), blueprintLinks.end(),
                        [removeId](const std::pair<int,int>& link) {
                            return (link.first / 100 == removeId) || (link.second / 100 == removeId);
                        }), blueprintLinks.end());
                    ImGui::PopID();
                    break;
                }

                // Simple connection UI
                if (!node.outputPins.empty()) {
                    ImGui::SetCursorPos(ImVec2(nodePos.x + 170, nodePos.y + 60));
                    ImGui::Text(">");
                }
                if (!node.inputPins.empty()) {
                    ImGui::SetCursorPos(ImVec2(nodePos.x + 10, nodePos.y + 60));
                    ImGui::Text("<");
                }

                ImGui::PopID();
            }

            // Simple connection line drawing
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            for (const auto& link : blueprintLinks) {
                // Find source and target nodes
                int sourceNodeId = link.first / 100;
                int targetNodeId = link.second / 100;

                ImVec2 sourcePos, targetPos;
                bool foundSource = false, foundTarget = false;

                for (const auto& node : blueprintNodes) {
                    if (node.id == sourceNodeId) {
                        sourcePos = ImVec2(node.position.x + 180, node.position.y + 60);
                        foundSource = true;
                    }
                    if (node.id == targetNodeId) {
                        targetPos = ImVec2(node.position.x + 20, node.position.y + 60);
                        foundTarget = true;
                    }
                }

                if (foundSource && foundTarget) {
                    ImVec2 canvasOffset = ImGui::GetCursorScreenPos();
                    drawList->AddLine(
                        ImVec2(canvasOffset.x + sourcePos.x, canvasOffset.y + sourcePos.y),
                        ImVec2(canvasOffset.x + targetPos.x, canvasOffset.y + targetPos.y),
                        IM_COL32(255, 255, 255, 255), 2.0f
                    );
                }
            }

            ImGui::EndChild();

            // Connection UI
            ImGui::Separator();
            static int connectFrom = -1, connectTo = -1;
            ImGui::Text("Connect Nodes:");
            ImGui::InputInt("From Node ID", &connectFrom);
            ImGui::SameLine();
            ImGui::InputInt("To Node ID", &connectTo);
            ImGui::SameLine();
            if (ImGui::Button("Connect")) {
                if (connectFrom > 0 && connectTo > 0) {
                    blueprintLinks.push_back({connectFrom * 100 + 1, connectTo * 100 + 1});
                }
            }

        } else {
            // CODE EDITOR MODE
            ImGui::Text("📝 .sp Script Code Editor");

            if (showBlueprintEditor) {
                ImGui::Text("Editing: %s", currentBlueprintPath.c_str());

                // Prepare char buffer for editing
                if (blueprintEditBuffer.size() < currentBlueprintCode.size() + 1024) {
                    blueprintEditBuffer.resize(currentBlueprintCode.size() + 1024);
                }
                std::copy(currentBlueprintCode.begin(), currentBlueprintCode.end(), blueprintEditBuffer.begin());
                blueprintEditBuffer[currentBlueprintCode.size()] = '\0';

                if (ImGui::InputTextMultiline("##code", blueprintEditBuffer.data(),
                    blueprintEditBuffer.size(), ImVec2(-1, 400),
                    ImGuiInputTextFlags_AllowTabInput)) {
                    currentBlueprintCode = std::string(blueprintEditBuffer.data());
                }
            } else {
                ImGui::Text("No file open for editing.");
                ImGui::Text("Create a Blueprint component to open the editor.");
            }
        }

        // Action buttons
        ImGui::Separator();
        if (ImGui::Button("💾 Save Blueprint/Code")) {
            if (visualMode) {
                GenerateBlueprintSP();
            } else {
                SaveCodeToFile();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("🔨 Compile to .sp")) {
            if (visualMode) {
                GenerateBlueprintSP();
                GenerateLuaFromSP();
            } else {
                SaveCodeToFile();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("⚡ Apply to Selected")) {
            ApplyScriptToSelected(registry, scripting);
        }

        // Clear blueprint
        ImGui::SameLine();
        if (ImGui::Button("🗑️ Clear")) {
            blueprintNodes.clear();
            blueprintLinks.clear();
            currentBlueprintCode.clear();
        }
    }
    ImGui::End();
}

void UnrealEditor::GenerateBlueprintSP() {
    // Generate .sp file from visual blueprint nodes
    std::filesystem::create_directories("assets/scripts/generated");

    if (currentBlueprintPath.empty()) {
        currentBlueprintPath = "assets/scripts/generated/blueprint_" + std::to_string(nextNodeId) + ".sp";
    }

    // Generate JSON-like .sp format
    std::ofstream ofs(currentBlueprintPath);
    ofs << "{\n";
    ofs << "  \"version\": \"1.0\",\n";
    ofs << "  \"type\": \"SproutBlueprint\",\n";
    ofs << "  \"nodes\": [\n";

    for (size_t i = 0; i < blueprintNodes.size(); ++i) {
        const auto& node = blueprintNodes[i];
        ofs << "    {\n";
        ofs << "      \"id\": " << node.id << ",\n";
        ofs << "      \"type\": \"" << node.type << "\",\n";
        ofs << "      \"name\": \"" << node.name << "\",\n";
        ofs << "      \"position\": [" << node.position.x << ", " << node.position.y << "],\n";
        ofs << "      \"params\": [\"" << node.param1 << "\", \"" << node.param2 << "\", \"" << node.param3 << "\"],\n";
        ofs << "      \"inputPins\": [";
        for (size_t j = 0; j < node.inputPins.size(); ++j) {
            ofs << node.inputPins[j];
            if (j < node.inputPins.size() - 1) ofs << ", ";
        }
        ofs << "],\n";
        ofs << "      \"outputPins\": [";
        for (size_t j = 0; j < node.outputPins.size(); ++j) {
            ofs << node.outputPins[j];
            if (j < node.outputPins.size() - 1) ofs << ", ";
        }
        ofs << "]\n";
        ofs << "    }";
        if (i < blueprintNodes.size() - 1) ofs << ",";
        ofs << "\n";
    }

    ofs << "  ],\n";
    ofs << "  \"connections\": [\n";

    for (size_t i = 0; i < blueprintLinks.size(); ++i) {
        const auto& link = blueprintLinks[i];
        ofs << "    {\"from\": " << link.first << ", \"to\": " << link.second << "}";
        if (i < blueprintLinks.size() - 1) ofs << ",";
        ofs << "\n";
    }

    ofs << "  ]\n";
    ofs << "}\n";
    ofs.close();

    AddLog("Generated .sp blueprint file: " + currentBlueprintPath, "Info");
}

void UnrealEditor::GenerateLuaFromSP() {
    // Generate Lua script from .sp blueprint
    std::string luaPath = currentBlueprintPath + ".lua";
    std::ofstream ofs(luaPath);

    ofs << "-- Generated Lua from SproutEngine Blueprint (.sp)\n";
    ofs << "-- Original file: " << currentBlueprintPath << "\n\n";

    // Generate variables
    for (const auto& node : blueprintNodes) {
        if (node.type == "Variable") {
            ofs << node.name << " = " << node.param1 << "\n";
        }
    }
    ofs << "\n";

    // Generate event functions
    for (const auto& node : blueprintNodes) {
        if (node.type == "Event") {
            if (node.name == "OnStart") {
                ofs << "function OnStart(id)\n";
                // Find connected nodes
                for (const auto& link : blueprintLinks) {
                    if (link.first / 100 == node.id) {
                        int targetNodeId = link.second / 100;
                        for (const auto& target : blueprintNodes) {
                            if (target.id == targetNodeId) {
                                if (target.type == "Function" && target.name == "Print") {
                                    ofs << "  Print(\"" << target.param1 << "\")\n";
                                } else if (target.type == "Function" && target.name == "SetRotation") {
                                    ofs << "  SetRotation(id, {" << target.param1 << ", " << target.param2 << ", " << target.param3 << "})\n";
                                }
                            }
                        }
                    }
                }
                ofs << "end\n\n";
            } else if (node.name == "OnTick") {
                ofs << "function OnTick(id, dt)\n";
                // Find connected nodes
                for (const auto& link : blueprintLinks) {
                    if (link.first / 100 == node.id) {
                        int targetNodeId = link.second / 100;
                        for (const auto& target : blueprintNodes) {
                            if (target.id == targetNodeId) {
                                if (target.type == "Function" && target.name == "SetRotation") {
                                    ofs << "  local x, y, z = GetRotation(id)\n";
                                    ofs << "  y = y + speed * dt\n";
                                    ofs << "  SetRotation(id, {x, y, z})\n";
                                }
                            }
                        }
                    }
                }
                ofs << "end\n\n";
            }
        }
    }

    ofs.close();
    AddLog("Generated Lua from .sp: " + luaPath, "Info");

    // Update current code for display
    std::ifstream ifs(luaPath);
    if (ifs) {
        currentBlueprintCode.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    }
}

void UnrealEditor::SaveCodeToFile() {
    if (!currentBlueprintPath.empty()) {
        std::ofstream ofs(currentBlueprintPath);
        ofs << currentBlueprintCode;
        ofs.close();
        AddLog("Saved code to: " + currentBlueprintPath, "Info");
    }
}

void UnrealEditor::ApplyScriptToSelected(entt::registry& registry, Scripting& scripting) {
    if (selectedEntity != entt::null && IsEntityValid(registry, selectedEntity)) {
        // Ensure Script component exists
        if (!registry.any_of<Script>(selectedEntity)) {
            registry.emplace<Script>(selectedEntity, currentBlueprintPath, 0.0, false);
        } else {
            auto& script = registry.get<Script>(selectedEntity);
            script.filePath = currentBlueprintPath;
        }

        // Try to load the Lua version if it exists
        std::string luaPath = currentBlueprintPath;
        if (luaPath.ends_with(".sp")) {
            luaPath += ".lua";
        }

        if (std::filesystem::exists(luaPath)) {
            scripting.loadScript(registry, selectedEntity, luaPath);
            AddLog("Applied script to entity: " + luaPath, "Info");
        } else {
            AddLog("No Lua file found. Compile blueprint first.", "Warning");
        }
    } else {
        AddLog("No selected entity to apply script to", "Warning");
    }
}
