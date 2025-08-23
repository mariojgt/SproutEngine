#pragma once
// Minimal no-op stub of imnodes API for build compatibility.
// If you later add real imnodes, remove this stub and link the library.
namespace imnodes {
inline void Initialize() {}
inline void Shutdown() {}
inline void BeginNodeEditor() {}
inline void EndNodeEditor() {}
inline void BeginNode(int) {}
inline void EndNode() {}
inline void BeginInputAttribute(int) {}
inline void EndInputAttribute() {}
inline void BeginOutputAttribute(int) {}
inline void EndOutputAttribute() {}
} // namespace imnodes
