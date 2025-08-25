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
