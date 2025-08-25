# RFC: Sprout Script Toolchain

This document outlines the proposed workflow for integrating the `.sp` scripting language into SproutEngine.

## Goals
- Author scripts in a dedicated editor with syntax highlighting and diagnostics.
- Transpile `.sp` sources to C++ headers and sources under `Generated/Cpp/`.
- Allow attaching generated components to entities and hot‑reloading them in the editor.

## Components
1. **Language Service** – lexer/parser/semantic checks used by the editor and CLI.
2. **`spc` CLI** – transpiles files, watches for changes and emits C++.
3. **Editor Integration** – in‑engine panel for editing scripts and attaching them to entities.
4. **Runtime Hooks** – optional helpers for reloading modules while the editor runs.

All pieces are guarded behind `SP_TOOLCHAIN_ENABLED` so existing builds remain unaffected.
