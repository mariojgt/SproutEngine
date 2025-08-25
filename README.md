# Sprout Engine (minimal)

A tiny, cross‑platform game editor skeleton inspired by Unreal — built on **Rust + Bevy**.
It gives you:
- A 3D viewport with camera controls
- A Hierarchy panel (select entities)
- An Inspector (edit position/rotation/scale)
- A super-simple **Visual Scripting** panel: add "blocks" like Rotate/Translate/Scale that run every frame

> Goal: keep it **simple** and hackable so you can grow features over time.

## Build & Run

### Prereqs
- Rust toolchain: https://rustup.rs
- On Linux: `sudo apt install libasound2-dev libudev-dev pkg-config` (or equivalents)
- On macOS: `xcode-select --install` (Command Line Tools)
- On Windows: install Rust + build tools (MSVC).

### Run
```bash
cargo run
```

If you get errors about crate versions, try:
```bash
cargo update
```

## Controls

**Viewport camera (orbit-style)**
- Right Mouse Drag: orbit
- Middle Mouse / Shift + Right Drag: pan
- Mouse Wheel: zoom
- F: frame selected entity

**UI**
- Left panel: Hierarchy (click to select)
- Right panel: Inspector (edit transform numerically)
- Bottom panel: Visual Script (add/remove/reorder simple ops)

## Project Layout

```
sprout_engine/
  ├─ Cargo.toml
  └─ src/
     └─ main.rs
```

## Notes
- This is intentionally minimal. Rendering, assets, gizmos, and node-graph scripting can be added later.
- For transform gizmos (move/rotate/scale handles), consider integrating ImGuizmo with an ImGui bridge or building one in egui.
