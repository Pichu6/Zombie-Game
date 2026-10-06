# 20 · Damage resolver

**Status:** Proposal. **Stage:** 7. **Needs:** [10](10-fortification.md), [12](12-body-wounds.md), [13](13-capabilities.md).

## Purpose

The first, small version of the one damage system for bodies and buildings ([0015](../../decisions/0015-tone-realistic-damage.md), [body trauma](../../design/body-trauma.md#one-damage-system)). Tier 0 needs melee strikes, bites, scratches, grabs and pushes on doors. Combat must be dangerous even against a few.

## State

| Field | Notes |
| --- | --- |
| load | energy (J) or force (N), contact area (mm²), sharpness (blunt, edge, point, teeth), direction, source |
| layer table | per tissue or material: thickness, energy to cut, energy to break; skin, fat, muscle, bone; wood, glass |
| clothing protection | per region: a simple cut and bite resistance value from worn items (jacket, jeans, boots) |

## Rules

1. Strike energy = strike force from the body ([13](13-capabilities.md)) × tool lever and mass factor × exertion; aim picks a region from a zone (head, torso, arms, legs) with a spread that widens with fear ([11](11-body-composition.md)), pain and exertion.
2. The load is spent layer by layer: each layer absorbs up to its threshold and produces a wound at the depth reached ([12](12-body-wounds.md)): blunt → contusion or fracture; edge → laceration; point → puncture; teeth → bite.
3. Walker attacks: a grab holds the target (escape uses strength and moving); while held, a bite attempt per few seconds; clothing on that region can stop a bite. A bite or scratch that breaks skin records a pathogen dose ([21](21-infection.md)).
4. Walker neutralisation (default origin): enough damage to the brain through the skull. Destroyed legs make it crawl; destroyed jaw stops bites; arms stop grabs ([threats](../../design/threats.md#localized-damage)).
5. Pushes and blows on doors and barricades go to [10](10-fortification.md) through the same load record.
6. Every hit is a sound event ([18](18-noise-field.md)) and an event with its cause ([25](25-chronicle.md)).
7. Realistic, not gore: the view shows only what the state says.

## Invariants

- Energy spent in layers ≤ energy delivered.
- A wound's depth never exceeds the layers the load reached.

## Tests

- A crowbar blow to the head from a strong pop neutralises a walker more often than from a weak one, over 50 seeds. (A3)
- A jacket lowers bite skin-break rate on the forearm, over 50 seeds. (A3)
- One pop vs three walkers in the open loses more often than wins, over 50 seeds. (A3)

## Calibration

| Value | Source |
| --- | --- |
| Human strike energies with tools | Biomechanics of hammer and bat swings (sports science literature); tune |
| Skull fracture energy | Forensic and impact biomechanics (e.g. Yoganandan et al.) |
| Human and animal bite forces | Bite-force studies (Braun et al.; dog-bite literature) |
| Fabric cut and puncture resistance | EN 388 / ASTM F1790 cut tests (as relative values) |

## Not in Tier 0

Firearms and ballistics, explosives, crushing by collapse, severed limbs as separate objects, fire.

## Model as implemented (Proposal, 2026-10-06)

Code: `Walkers::strike`, `Walkers::bite`, `Walkers::attack` in `engine/sim/src/walkers.cpp`; the `strike_walker` process (`data/tier0/processes.toml`, effect `hit`).

- **Strike:** energy = 25 J × (push force / 500 N) × (1 + 3 × tool), tool = best of blunt, 0.9 × lever, 0.6 × edge. Aim hits the head with 0.6 × (1 − 0.5 fear) × √exertion, else the chest. A head hit delivers energy × U(0.6, 1.2); ≥ 60 J destroys the brain (death `BrainDestroyed`, census `Destroyed`); ≥ 36 J leaves a fracture, less a contusion. Average adult with a crowbar ≈ 100 J; in 50 strikes the strongest neighbour put walkers down 30 times, the weakest 6 (test).
- **Walker attack** every 3 s when in reach: grab 0.5; held victims escape with 0.2 × strength ratio, else a bite with 0.4. After a bite the walker worries it for 30 s before biting again. At most 3 walkers get at one person; the rest crowd behind.
- **Bite wound:** region weighted to forearms and hands (neck 8 %), depth skin 0.3 / fat 0.35 / muscle, 25–45 mm, contamination 0.9, pathogen dose 1. A bite wakes a sleeper.

**Known gaps:** two zones only (head, chest); no layer-by-layer spend; no clothing; no jaw, arm or leg destruction on walkers; no scratches; strikes make no noise yet; the scripted pop has no flee or shove.
