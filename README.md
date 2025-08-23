# SproutEngine (C++ mini “Unreal-like” starter)

This repo gives you:
- Window + ## Roadmap (towards Unreal-like workflow)

### ✅ Blueprint-like visual scripting (COMPLETED!)
- **Interactive node-based editor** similar to Unreal Engine's Blueprint system
- **Drag-to-connect functionality**: Drag from output pins to input pins to create connections
- **Professional node types**: Events (red), Actions (green), Math (blue), Components (yellow)
- **Node manipulation**: Left-click to select, drag to move, Delete key to remove nodes
- **Real-time execution**: "Run Blueprint" button to test blueprint logic flow
- **C++ code generation**: Blueprints compile to efficient C++ code
- **Script Editor (.sp)**: Text-based scripting with syntax highlighting and C++ compilation
- **Dual workflow**: Both visual blueprints (.bp) and text scripts (.sp) compile to same C++ output

### Scenes
- JSON save/load of entity registry to `assets/scenes/*.json`.
- Menu entries: **File → New / Open / Save**.

### Drag-and-drop assets
- Drop models (`.gltf/.glb`) from Finder/Explorer into viewport to spawn entities.
- Planned: tinygltf integration in `vcpkg.json`.

### HUD / Widgets
- UI canvas rendered after 3D pass.
- Widgets: Button, Text, ProgressBar.
- Exposed to Lua (`UI:Find("WidgetName")`).enGL + GLAD)
- ECS with EnTT
- Editor UI with Dear ImGui (docking-ready structure)
- Visual graph stub (imnodes) for future “Blueprint”-style logic
- Lua scripting via sol2 (hot-reload on file save)
- A demo cube entity rotated by `assets/scripts/Rotate.lua`

---

## Requirements
- **CMake 3.22+**
- **C++20 compiler** (MSVC 2022 / Clang 15+ / GCC 12+)
- **vcpkg** (manifest mode)
- GPU with **OpenGL 3.3+**

> Always pass `-DCMAKE_TOOLCHAIN_FILE=…/vcpkg.cmake` so CMake uses the manifest (`vcpkg.json` at repo root).

---

## Windows (MSVC, x64)
1. Install **Visual Studio 2022** with “Desktop development with C++”.
2. Install vcpkg:
   ```powershell
   git clone https://github.com/microsoft/vcpkg $env:USERPROFILE\vcpkg
   & $env:USERPROFILE\vcpkg\bootstrap-vcpkg.bat
   ```
3. Configure & build:
   ```powershell
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
     -DCMAKE_TOOLCHAIN_FILE=$env:USERPROFILE\vcpkg\scripts\buildsystems\vcpkg.cmake `
     -DCMAKE_BUILD_TYPE=RelWithDebInfo
   cmake --build build --config RelWithDebInfo
   ```
4. Run:
   ```powershell
   .\build\RelWithDebInfo\SproutEngine.exe
   ```

---

## macOS (Apple Clang)
```bash
brew install cmake ninja
git clone https://github.com/microsoft/vcpkg ~/vcpkg && ~/vcpkg/bootstrap-vcpkg.sh

rm -rf build
cmake -S . -B build -G Ninja   -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
./build/SproutEngine
```

> Note: macOS deprecates OpenGL but still ships it; 3.3 core profile is used. Ignore deprecation warnings.

---

## Linux (Ubuntu example)
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build xorg-dev libglu1-mesa-dev
git clone https://github.com/microsoft/vcpkg ~/vcpkg && ~/vcpkg/bootstrap-vcpkg.sh

rm -rf build
cmake -S . -B build -G Ninja   -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
./build/SproutEngine
```

---

## Run
After build, run the produced binary. A window opens with a spinning cube.

- Use the **Hierarchy** to select entities.
- In **Inspector**, edit Transform, add the `Script` component, or change the script path.
- Modify `assets/scripts/Rotate.lua` while the app runs — it hot-reloads on save.

### 🎨 Blueprint Editor Usage
1. **Open**: Go to `View → Blueprint Editor`
2. **Add Nodes**: Use the Node Library on the left - Events (red), Actions (green), Math (blue), Components (yellow)
3. **Connect Nodes**: **Drag from output pins** (right side) to **input pins** (left side) - like Unreal Engine!
4. **Move Nodes**: Left-click and drag nodes to reposition them
5. **Select & Delete**: Left-click to select nodes, press **Delete** key to remove them
6. **Test Blueprint**: Click **"Run Blueprint"** to execute and see results in Console
7. **Generate C++**: Click **"Compile to C++"** to see generated C++ code

### 📝 Script Editor Usage
1. **Open**: Go to `View → Script Editor`
2. **Write Code**: Create .sp scripts with C++-like syntax
3. **Live Preview**: See generated C++ code in real-time
4. **Compile**: Both .sp scripts and .bp blueprints compile to the same efficient C++ output

---

## Roadmap (towards Unreal-like workflow)

### Scenes
- JSON save/load of entity registry to `assets/scenes/*.json`.
- Menu entries: **File → New / Open / Save**.

### Drag-and-drop assets
- Drop models (`.gltf/.glb`) from Finder/Explorer into viewport to spawn entities.
- Planned: tinygltf integration in `vcpkg.json`.

### Blueprint-like visual scripting
- imnodes-based graph → compiled to Lua file under `assets/scripts/generated/`.
- Attach generated script to the entity’s `Script` component.
- Node palette: Tick, OnBeginPlay, Transform ops, Math, Branch, Variables.

### HUD / Widgets
- UI canvas rendered after 3D pass.
- Widgets: Button, Text, ProgressBar.
- Exposed to Lua (`UI:Find("WidgetName")`).

---

## Notes
- ✅ **Visual scripting system COMPLETE**: Full Blueprint Editor with drag-to-connect, node deletion, and C++ compilation
- ✅ **Script Editor COMPLETE**: Text-based .sp scripting with syntax highlighting and C++ generation
- Renderer is intentionally minimal (OpenGL 3.3). Swap to bgfx/Vulkan later.
- Asset import is not included yet (meshes are cubes). Add tinygltf + PBR next.

---

## Troubleshooting
- **CMake can’t find packages**
  Ensure vcpkg is in **manifest mode** and you pass the `-DCMAKE_TOOLCHAIN_FILE` flag.
- **ImGui target mismatch**
  Some vcpkg triplets use `unofficial-imgui::imgui`. Adjust your `CMakeLists.txt` if needed.
- **Lua linkage**
  If `Lua::Lua` target isn’t present, fallback `${LUA_LIBRARIES}` is used.
- **GL loader issues**
  If GLAD isn’t found as `glad::glad`, link `glad` directly.
- **macOS warnings**
  Use `-DGL_SILENCE_DEPRECATION` to hide OpenGL warnings.

---

Happy hacking!
