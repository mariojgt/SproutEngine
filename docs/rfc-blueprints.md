# Blueprint System RFC

## Overview
This document proposes a feature-flagged Blueprint system for SproutEngine. The
system mirrors Unreal Engine's visual scripting workflow and is delivered in
incremental stages. A new CMake option `BLUEPRINTS_ENABLED` controls whether the
feature is compiled and exposed in the editor.

## Goals
- Visual node graph editor integrated into the existing ImGui-based tools.
- Deterministic graph compiler that emits C++ headers/sources plus source maps.
- Hot‑reload bridge that rebuilds only changed blueprints and updates the running
  engine.
- Debugging support: breakpoints, execution tracing, and value watches.
- All functionality lives behind `BLUEPRINTS_ENABLED` (off by default).

## Roadmap
1. Graph core library and validator.
2. C++ code generation and CLI (`bpc`).
3. Editor window with palette, inspector, and undo/redo.
4. Asset workflow and Add Component flow.
5. Hot‑reload and runtime debugging hooks.
6. Documentation, migration utilities, and performance polish.

## Repository Additions (planned)
```
/docs/
  rfc-blueprints.md                 # this file
  blueprints-user-guide.md
  blueprints-graph-spec.md
  blueprints-engine-integration.md
/tools/
  bpc/
  bp-lang/
/engine/
  editor/windows/BlueprintEditor.cpp/.h
  runtime/blueprints/
/game/Blueprints/...
```

## Feature Flag
The build system now exposes `BLUEPRINTS_ENABLED`. Future patches will wrap
Blueprint-related code with this macro and expand the folders above.
