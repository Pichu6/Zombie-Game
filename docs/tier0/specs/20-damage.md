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

1. Strike energy = strike force from the body ([13](13-capabilities.md)) × tool lever and mass factor × exertion; aim picks a region from a zone (head, torso, arms, legs) with a spread that widens with fear, pain and exertion.
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

- A crowbar blow to the head from a strong pop neutralises a walker more often than from a weak one. (A2)
- A jacket lowers bite skin-break rate on the forearm. (A2)
- One pop vs three walkers in the open loses more often than wins. (A2)

## Calibration

| Value | Source |
| --- | --- |
| Human strike energies with tools | Biomechanics of hammer and bat swings (sports science literature); tune |
| Skull fracture energy | Forensic and impact biomechanics (e.g. Yoganandan et al.) |
| Human and animal bite forces | Bite-force studies (Braun et al.; dog-bite literature) |
| Fabric cut and puncture resistance | EN 388 / ASTM F1790 cut tests (as relative values) |

## Not in Tier 0

Firearms and ballistics, explosives, crushing by collapse, severed limbs as separate objects, fire.
