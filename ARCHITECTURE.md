# Sprout Engine - Enhanced Project Structure

This document outlines the modular architecture for Sprout Engine as it evolves into a full game engine.

## Crate Structure

```
sprout_engine/
├── Cargo.toml                    # Workspace root
├── crates/
│   ├── sprout_core/              # Core engine systems
│   ├── sprout_editor/            # Editor-specific code
│   ├── sprout_render/            # Advanced rendering
│   ├── sprout_physics/           # Physics integration
│   ├── sprout_audio/             # Audio systems
│   ├── sprout_animation/         # Animation systems
│   ├── sprout_scripting/         # Visual scripting
│   ├── sprout_assets/            # Asset pipeline
│   ├── sprout_ui/                # Game UI systems
│   └── sprout_tools/             # Development tools
├── examples/                     # Example projects
├── templates/                    # Project templates
├── docs/                         # Documentation
└── assets/                       # Shared assets
```

## Module Responsibilities

### sprout_core
- ECS extensions
- Scene management
- Resource management
- Platform abstraction
- Core utilities

### sprout_editor
- Editor UI and windows
- Viewport management
- Gizmo systems
- Project management
- Asset browser

### sprout_render
- Advanced rendering pipeline
- Material system
- Lighting
- Post-processing
- Shader management

### sprout_physics
- Physics integration (Rapier)
- Collision detection
- Rigid body dynamics
- Soft body physics

### sprout_audio
- 3D spatial audio
- Audio processing
- Music systems
- Sound effects

### sprout_animation
- Animation state machines
- Skeletal animation
- IK solvers
- Animation blending

### sprout_scripting
- Visual scripting system
- Blueprint-like editor
- Script compilation
- Hot reloading

### sprout_assets
- Asset import pipeline
- Asset processing
- Format converters
- Asset database

### sprout_ui
- Game UI framework
- Widget system
- Layout management
- UI animations

### sprout_tools
- Build tools
- Asset processors
- Optimization tools
- Debugging utilities
