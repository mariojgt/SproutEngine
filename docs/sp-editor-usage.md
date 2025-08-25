# Sprout Script Editor Usage

When `SP_TOOLCHAIN_ENABLED` is defined the editor exposes a **Scripts** panel.
From here you can:

1. Create a new `.sp` script or open an existing one.
2. Edit the script with syntax highlighting and diagnostics provided by the in‑process language service.
3. Press **Save** to transpile the file to C++ and hot‑reload the generated component.

When an entity with a `Script` component is selected the **Inspector** shows an **Edit Script** button which opens the same editor for that file.

The generated C++ sources are written to `Game/Generated/Cpp/` and picked up by the build system on the next compile.
