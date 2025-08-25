# Sprout Engine — Unified (glTF + FBX + Animator)

**What’s inside**
- File: New / Save / Load (JSON scene)
- Content Browser: scans `/assets` for `.gltf/.glb/.fbx/.png/.jpg`
- Viewport click-select + W/E/R gizmos (X/Y/Z axis lock)
- Inspector: edit transform numerically
- Visual Scripting: list + simple Node Graph
- **Animator tab:** attach an Animator to an entity, list its clips, create simple state machine with a single float parameter `speed` and thresholds
- **FBX support:** via `bevy_mod_fbx` (FBX 7.4/7.5 binary recommended). Anim support depends on the plugin’s current capabilities.

## Build
```
cargo run
```
If crates mismatch: `cargo update`. If `bevy_mod_fbx` complains about Bevy version, check its README for matching versions and adjust.

## Assets
Put models/textures in the `assets/` folder (next to `Cargo.toml`). FBX works best as Binary 7.4/7.5. If FBX fails, export glTF/GLB as a fallback.
