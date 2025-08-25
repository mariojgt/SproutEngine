# Sprout Engine — Open Source Game Engine Alternative

**A free, open-source game engine built on Bevy that aims to rival Game Engine in features and usability.**

## 🎯 Vision
Create a comprehensive, user-friendly game engine that provides:
- Professional-grade tools for game development
- Modern ECS architecture (Entity Component System)
- Cross-platform support (Windows, macOS, Linux, Web, Mobile)
- Visual scripting system similar to Unreal's Blueprints
- Advanced rendering pipeline with PBR materials
- Integrated physics, audio, and animation systems

## 🚀 Current Features

### Editor
- **Multi-viewport system** with Perspective, Top, Front, and Right views
- **Advanced gizmo system** with translate, rotate, scale tools
- **Material editor** with PBR workflow and texture management
- **Project management** with settings and build configurations
- **Asset browser** with support for glTF, FBX, textures, and audio
- **Scene hierarchy** with parent-child relationships
- **Inspector panel** for component editing
- **Visual scripting** foundation (Blueprint-like system in development)

### Rendering
- PBR (Physically Based Rendering) materials
- Real-time lighting and shadows
- Post-processing effects
- Texture compression and optimization
- Multi-platform graphics (Metal, Vulkan, DirectX, WebGL)

### Asset Pipeline
- glTF/GLB support (recommended)
- FBX support via `bevy_mod_fbx`
- Texture import with various formats
- Audio asset management
- Material library system

### Animation
- Skeletal animation support
- Animation state machines
- Basic animator with parameter-driven transitions
- Animation blending (in development)

## 🛠️ Quick Start

### Prerequisites
- Rust (latest stable version)
- Git

### Build and Run
```bash
git clone https://github.com/mariojgt/SproutEngine.git
cd SproutEngine
cargo run
```

### Creating Your First Project
1. Launch Sprout Engine
2. Go to **Project → New Project...**
3. Enter your project details
4. Start building your game!

## 📁 Project Structure
```
sprout_engine/
├── src/
│   ├── main.rs              # Main engine entry point
│   ├── viewport.rs          # Multi-viewport system
│   ├── gizmo.rs            # Advanced gizmo tools
│   ├── material.rs         # Material editor
│   ├── project.rs          # Project management
│   └── ...
├── assets/                 # Default engine assets
├── examples/               # Example projects
├── templates/              # Project templates
├── docs/                   # Documentation
├── ROADMAP.md             # Development roadmap
└── ARCHITECTURE.md        # Technical architecture
```

## 🗺️ Development Roadmap

### Phase 1: Core Editor (Current → 3 months)
- [x] Multi-viewport system
- [x] Advanced gizmos
- [x] Material editor foundation
- [x] Project management
- [ ] Dockable UI panels
- [ ] Undo/Redo system
- [ ] Enhanced asset pipeline

### Phase 2: Advanced Rendering (3-6 months)
- [ ] Deferred rendering pipeline
- [ ] Global illumination
- [ ] Advanced lighting system
- [ ] Post-processing stack
- [ ] Terrain rendering

### Phase 3: Animation & Physics (6-9 months)
- [ ] Advanced animation system
- [ ] Physics integration (Rapier)
- [ ] Character controllers
- [ ] AI navigation

### Phase 4: Visual Scripting (9-12 months)
- [ ] Blueprint-style visual scripting
- [ ] Node-based editor
- [ ] Runtime compilation
- [ ] Debugging tools

See [ROADMAP.md](ROADMAP.md) for the complete development plan.

## 🤝 Contributing

We welcome contributions! Whether you're:
- A Rust developer
- A game developer with UX insights
- A technical artist
- A documentation writer
- A tester

Your help is valuable. Check out our [contribution guidelines](CONTRIBUTING.md) to get started.

### Areas Needing Help
- **Rendering**: Advanced graphics features
- **UI/UX**: Editor interface improvements
- **Documentation**: Tutorials and guides
- **Testing**: Cross-platform testing
- **Examples**: Sample projects and templates

## 📖 Documentation

- [Getting Started Guide](docs/getting-started.md)
- [API Reference](docs/api-reference.md)
- [Tutorial Series](docs/tutorials/)
- [Architecture Overview](ARCHITECTURE.md)
- [Development Roadmap](ROADMAP.md)

## 🎮 Example Projects

Check out the `examples/` directory for sample projects:
- **Basic 3D Scene**: Simple scene with lighting and materials
- **Platformer Game**: 2D/3D platformer mechanics
- **First Person Controller**: FPS-style character controller
- **Material Showcase**: PBR material examples

## 🔧 Technical Stack

- **Engine Core**: [Bevy](https://bevyengine.org/) (Rust ECS game engine)
- **Graphics**: wgpu (cross-platform graphics)
- **Physics**: Rapier (Rust physics engine)
- **UI**: egui (immediate mode GUI)
- **Audio**: Bevy Audio + advanced processing
- **Scripting**: Rust + Visual scripting system

## 📊 Performance Goals

- 60+ FPS for complex scenes
- Sub-10ms frame times (VR ready)
- Fast iteration times with hot reloading
- Scalable from indie to AAA projects
- Memory efficient asset streaming

## 📄 License

Sprout Engine is free and open source under the [MIT License](LICENSE).

## 🌟 Support the Project

If you find Sprout Engine useful:
- ⭐ Star this repository
- 🐛 Report bugs and suggest features
- 🔄 Share with other developers
- 💖 Consider sponsoring development

---

**Join our community:**
- [Discord Server](https://discord.gg/sprout-engine)
- [Reddit Community](https://reddit.com/r/SproutEngine)
- [Twitter Updates](https://twitter.com/SproutEngine)

*Making game development accessible to everyone.*nified (glTF + FBX + Animator)

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
