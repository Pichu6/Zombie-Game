# 0001 · Our own engine, from scratch

- **Date:** 2026-10-05
- **Status:** Decided by the user

## Decision

The game is built from scratch on our own C++ engine with a Vulkan renderer. The existing Godot project (Godot, Mixamo, Blender, Tripo3D) is a reference for ideas only.

## Why

The user wants to innovate in deep simulation without the constraints of a general-purpose engine.

## Consequences

- We write *this game's* engine, not a general engine.
- External authoring tools (Blender or others) are still fine for modelling; the engine does not depend on them at runtime.
