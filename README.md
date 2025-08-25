# Sprout Engine (Unified)

Cross‑platform minimal editor starter with:
- File: **New / Save / Load** (JSON)
- **Gizmos**: W/E/R + X/Y/Z, drag to move/rotate/scale
- **Click-to-select in viewport**
- **Node Graph** (basic) + list-based visual script
- **Content Browser** scans `/assets` for `.gltf/.glb/.png/.jpg` and spawns them

## Run
```
cargo run
```
If crates whine: `cargo update`

## Usage
- Drop files into the `assets/` folder (next to Cargo.toml).
- Open the app → **Content Browser** (bottom) → select a file → **Spawn**.
- Left‑click an object in the viewport to **select** it, then use gizmos (W/E/R + X/Y/Z), or edit in Inspector.
- Save/Load scene via **File** menu (JSON path field).

## Notes
- Picking uses simple ray‑sphere tests (fast, good enough for prototypes). For perfect picking per‑mesh triangles, integrate a picking plugin later.
- glTF spawns the default scene of the file at origin; images spawn as unlit quads.
