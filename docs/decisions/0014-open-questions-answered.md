# 0014 · Open questions answered: world simulation, engine stack, clothing, quality

- **Date:** 2026-10-05
- **Status:** Decided by the user ("yes at suggested" to every recommendation).
- **Decides:** the proposals in [0002](0002-engine-stack.md) and [0003](0003-clothing-wrap.md), the formula left open in [0006](0006-crafting-quality.md), and the world simulation questions from [vision](../vision.md).

## World simulation

1. Movement speed belongs to the path: body speed inside the local zone, real-world speeds between towns.
2. Outbreak odds (will it fizzle) are hidden while playing an individual; shown in the god view and after death.
3. Arrival is drawn narrowly around day 7.
4. Default origin: a hidden bite, 1–2 days to turn.
5. Default start role: civilian; Responder (real local health officer or mayor) is an option.
6. The game pauses while giving orders in City and Macro.
7. Instant build is a sandbox option only.
8. Orders need working communications to reach people.
9. Presets that break realism are rejected outside Sandbox and tagged inside it.

## Earlier proposals

- **Engine stack:** as proposed in [0002](0002-engine-stack.md): own simulation core and small Vulkan renderer; SDL3, Jolt, ozz-animation, Recast/Detour, fastgltf or cgltf, Dear ImGui, miniaudio and Steam Audio. The simulation core runs and is tested without a window.
- **Clothing:** GPU wrap as proposed in [0003](0003-clothing-wrap.md); tight clothing hides the skin it covers (except wounds and the anatomy view).
- **Quality formula:** `Q_out` = weighted mean of input Q (weights by function), capped by `min(skill, Q_tool, method ceiling, Q_station)`, with the spread set by process stability. Method ceilings and stability as in [items and crafting](../design/items-crafting.md#production-ladder) and [0013](0013-production-ladder.md). Exact numbers per family remain to be tuned.

## Still open

Tone and level of graphic violence.
