# Engine Integration

The `.sp` workflow plugs into the existing runtime through a transpile step.

## Build Pipeline
1. The `spc` CLI scans `Game/Scripts/*.sp`.
2. For each file it emits matching `.hpp` and `.cpp` files under `Game/Generated/Cpp/`.
3. The regular C++ build then compiles these generated sources.

## Hot Reload
During development the editor watches for file saves. When a script changes it is re‑transpiled and the engine reloads the corresponding component if possible.

## Configuration
Projects opt‑in by adding a `.spconfig.json` file at the game root. This file lists source directories and output paths.
