# 0002 · Engine stack

- **Date:** 2026-10-05
- **Status:** Proposal (from the deep-simulation research; not yet confirmed by the user)

## Proposal

| Area | Our own | Library |
| --- | --- | --- |
| Simulation core | Yes, this is the innovation | None |
| Renderer | Yes, small: clustered forward, skinning, morphs and clothing wrap in compute | vk-bootstrap, Vulkan Memory Allocator, meshoptimizer |
| Window and input | No | SDL3 |
| Physics and collision | No | Jolt Physics |
| Skeletal animation | Thin layer | ozz-animation |
| Navigation | No | Recast and Detour |
| Asset import | Converter to our own format | cgltf or fastgltf |
| Tools and editor | Yes, inside the engine | Dear ImGui |
| Audio | No | miniaudio, Steam Audio for occlusion |

## Architecture rule

The simulation core is a library with no renderer dependency, which runs and is tested without a window.
