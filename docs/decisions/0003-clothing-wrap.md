# 0003 · Clothing via GPU wrap

- **Date:** 2026-10-05
- **Status:** Proposal

## Proposal

Clothing has no shape keys and no per-garment fitting. Each garment vertex stores a binding to the body surface (triangle, barycentrics, offset in a local frame). A compute shader re-places it every frame after the body's morph and skinning. Details with relief (pockets, flaps, straps) are modules bound to the base garment. Dirt and blood live in masks over a shared body UV.

## Requirement

All bodies share one topology, one UV layout and one skeleton, and change through morphs.

## Open

Does tight clothing hide the skin it covers (except for wounds or the anatomy view)? Recommendation: yes.

Details: [docs/research/clothing-wrap.md](../research/clothing-wrap.md)
