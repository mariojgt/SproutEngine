# Sprout Engine (starter)

Cross‑platform C++20 starter for a lightweight game engine, using **SDL2** for window/input and **bgfx** for rendering (Direct3D/Metal/Vulkan/OpenGL underneath). This repo is set up to build on **Windows**, **macOS**, and **Linux** using **CMake**. Dependencies are resolved via **vcpkg** manifest mode.

> First run draws a blank window with bgfx initialized and debug text "Sprout Engine running."

---

## 1) Prereqs

- CMake 3.22+
- A C++20 compiler
  - Windows: MSVC (Visual Studio 2022) or LLVM/Clang + Ninja
  - macOS: Xcode Command Line Tools (Clang)
  - Linux: GCC or Clang
- Git
- **vcpkg** (recommended for dependencies)

### Install vcpkg (one‑time)

```bash
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh    # macOS/Linux
# or
.cpkg\bootstrap-vcpkg.bat  # Windows
```

You can set an environment variable `VCPKG_ROOT` pointing to that folder (optional).

---

## 2) Configure & Build (vcpkg manifest mode)

From the repo root:

### macOS / Linux
```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build -j
./build/bin/SproutDemo
```

### Windows (Developer PowerShell)
```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" -A x64
cmake --build build --config Release
# Run (from build/bin/Release):
build\bin\Release\SproutDemo.exe
```

> If you don't set `VCPKG_ROOT`, replace it with the absolute path to your vcpkg clone.

---

## 3) What you get

```
engine/
  Core/        # Application lifecycle
  Platform/    # SDL2 window + input
  Render/      # bgfx setup + per‑frame calls
samples/
  Triangle/    # Minimal app that opens a window and renders the bgfx view
```

The **next steps** are to add ECS, a camera, and mesh loading via glTF. The skeleton is ready for those modules.

---

## 4) Troubleshooting

- If CMake can't find packages:
  - Ensure you passed the **vcpkg toolchain** arg.
  - Ensure the triplet is correct (e.g., default `x64-osx`, `x64-windows`, `x64-linux`).
- On macOS:
  - bgfx uses **Metal** by default; frameworks are linked in CMake.
- On Linux:
  - Make sure you have graphics dev packages (e.g., X11/Wayland dev libs) if your distro requires them. vcpkg takes care of most.
- On Windows:
  - If you see unresolved `SDL_main`, make sure `SDL2::SDL2main` is linked (already handled in CMake).

---

## 5) License

MIT.

---

## Update v0.2.0 — ECS + Camera + glTF loader

- Added **EnTT** (ECS) and **glm** (math).
- Added **tinygltf** loader and a sample asset: `assets/Box.gltf`.
- Demo creates a Scene with a Camera and loads `Box.gltf` (prints mesh stats in console).

> Rendering the mesh will come next (requires bgfx shader program setup). For now, Sprout initializes bgfx, sets up ECS, loads mesh data, and runs a frame loop.

### Run the demo

Use the same build instructions as before, then run `SproutDemo`. Check your terminal output for something like:

```
[Sprout] Loaded glTF: assets/Box.gltf (vertices: ####, indices: ####)
```


---

## Update v0.3.0 — Mesh Rendering

- Adds a minimal bgfx shader pipeline (solid color) compiled via **bgfx shaderc**.
- Renders any `ECS::Mesh` using transient vertex/index buffers.
- Uses the primary camera's view/projection.
- Sample scene loads `assets/Box.gltf` and draws it.

### Shader compilation (automatic if `shaderc` is found)

CMake tries to locate `shaderc` (from bgfx tools via vcpkg) and compiles:
- `shaders/vs_solid.sc` (vertex)
- `shaders/fs_solid.sc` (fragment)

Artifacts are placed under `build/shaders/<backend>/`. At runtime, Sprout loads the matching backend folder:
- Windows → `d3d11`
- macOS → `metal`
- Linux → `glsl` (GL 4.3)

If CMake can’t find `shaderc`, the build still succeeds but meshes won't draw; you’ll only see a clear color and debug text. Install `shaderc` from vcpkg or ensure it’s on PATH.


---

## Update v0.4.0 — Fly Cam, Light, Lua Scripts

**New features**
- **Fly camera**: Hold Right Mouse to capture the mouse, use **W/A/S/D** to move, **Q/E** to move down/up. Scroll to change speed.
- **Directional light** and a **lit shader** (Lambert) replacing solid color.
- **Lua scripting** with **sol2**:
  - Add a Script component with a file path (e.g., `scripts/rotator.lua`).
  - Engine calls `OnStart(entity)` once and `OnUpdate(entity, dt)` every frame.
  - Scripts can read/write Transform (`position`, `rotation`, `scale`) via bindings.

**Code editor**
- For now, Sprout offers a simple action to open a script file in your OS default editor.
  - Press **F2** to open `scripts/rotator.lua` (if attached) in your editor (`open` on macOS, `start` on Windows, `xdg-open` on Linux).

> Note: A full in-engine text editor UI (ImGui) can be added later.

