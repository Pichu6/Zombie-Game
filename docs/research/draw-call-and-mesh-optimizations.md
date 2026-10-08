# Draw-call and mesh optimizations, from a voxel-engine video

**Date:** 2026-10-08. **Status:** research note, not a decision. **Feeds:** [engine architecture](../design/engine-architecture.md), [0002](../decisions/0002-engine-stack.md). **Evidence:** one video, "I optimised my game engine up to 12000 FPS" (YouTube, `https://www.youtube.com/watch?v=40JzyaOYJeY`), read from the transcript the user pasted; the video's code was not read. It is about an OpenGL voxel world and its frame rates come from an empty demo scene, so they show relative gains only and say nothing about our game. **IDs:** DMO.

The user shared the video and asked if it is useful. It is not needed now (there is no 3D renderer yet, and no vertical-slice scenario asks for render performance), so the useful parts are kept here for when the Vulkan renderer starts ([0002](../decisions/0002-engine-stack.md), [0055](../decisions/0055-defaults-for-fixes-from-engineering-research.md) §1). The [renderer](renderer.md) note covers the same ground from primary sources; this note only adds the video's step-by-step numbers.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| DMO-1 | Chunks of 32³ voxels share one mesh; faces between two solid voxels are skipped and equal coplanar faces merged ("greedy meshing"): 115 M triangles to 1.2 M, 50 to 2,500 FPS. | reported | the video |
| DMO-2 | A vertex packed in 32 bits (chunk-local position 6 bits per axis, face direction 3 bits, texture id 7 bits), unpacked in the vertex shader by shift and mask, with the chunk's world position passed separately: 24 MB to 3.4 MB, 4,000 FPS. | reported | the video |
| DMO-3 | Instancing one 4-vertex face as a triangle strip, with one 32-bit record per face instead of six vertices (the shader rotates and stretches it by the merged length): 3.4 MB to 580 KB, 6,500 FPS. | reported | the video |
| DMO-4 | One big buffer for all chunks, an indirect buffer of draw commands (`glMultiDrawArraysIndirect`) and a storage buffer of per-chunk data indexed by `gl_DrawID`: 12,000 FPS. | reported | the video |
| DMO-5 | Six meshes per chunk, one per face direction, drawing only the directions that can face the camera (culling before the vertex shader): 14,000 FPS. | reported | the video |
| DMO-6 | Moving the face direction from the vertex to the per-mesh storage buffer frees bits for a second merged length (2D greedy merging): 79 k triangles, 17,000 FPS. | reported | the video |
| DMO-7 | GPU-driven drawing (one buffer, indirect commands, per-draw records read by draw index) is the main idea for us: in Vulkan `vkCmdDrawIndexedIndirect` or `...IndirectCount`, as niagara does ([RN-16](renderer.md)). It keeps the CPU for the simulation, where our cost goes. | estimate | DMO-4, [RN-16](renderer.md) |
| DMO-8 | Instancing fits crowds of walkers and repeated props (furniture, debris, wall modules) once crowds are on screen ([RN-32](renderer.md)). | estimate | DMO-3, [RN-32](renderer.md) |
| DMO-9 | Hidden-face removal and merging fit building interiors baked from modular walls and floors; meshoptimizer covers simplification and vertex order ([RN-20](renderer.md)). Medium value. | estimate | DMO-1, [RN-20](renderer.md) |
| DMO-10 | Compact vertices fit grid-aligned building geometry (small integer positions from a chunk origin, packed normals), not skinned bodies; a later tuning step ([RN-15](renderer.md)). | estimate | DMO-2, [RN-15](renderer.md) |
| DMO-11 | Culling by face direction is specific to axis-aligned voxel faces; frustum and occlusion culling of whole objects matter more for us. Low value. | estimate | DMO-5 |

## What applies to us

Our world is not made of voxels: walls, buildings, items and bodies are ordinary meshes, and most of the world is simulated without being drawn. So we take the general ideas (DMO-7 to DMO-10), not the voxel algorithms.

## When to come back to this

When the Vulkan renderer gets a vertical-slice scenario that needs it, for example many walkers on screen at once. Until then this note is reference only.
