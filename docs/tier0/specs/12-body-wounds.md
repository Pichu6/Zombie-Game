# 12 · Body: regions and wounds

**Status:** Implemented (stage 3; numbers are Proposal). **Stage:** 3. **Needs:** [11](11-body-composition.md).

## Purpose

17 body regions with tissue layers, a wound list per region, bleeding and bacterial infection. This carries "a scratch that gets infected", "you bled out" and the bite that hides an infection.

## State

| Field | Unit | Notes |
| --- | --- | --- |
| regions | 17 | head, neck, chest, abdomen, pelvis; upper arm, forearm, hand, thigh, shin, foot × left/right |
| tissue per region | skin, fat (mm, from fat kg and distribution), muscle (mm), bone integrity | Fat and muscle share derived from [11](11-body-composition.md) |
| wound | type, region, depth layer, length (mm), bleed rate (mL/min), contamination (0–1), cleaned, dressing (instance with Q and condition), infection stage, pathogen dose | Many per region |
| blood volume | mL | ~70 mL/kg lean-adjusted; part of structure mass |
| pain | 0–1 per region and total | Derived from the wounds each time it is read; never stored or accumulated |

Wound types in Tier 0: scratch, laceration, puncture, bite, blunt contusion, fracture.

## Rules

1. Bleed rate is set by depth and region (vessels), falls as a clot forms, rises again with movement of that region, and drops with pressure or a dressing.
2. Blood loss moves blood to a sink; consciousness and movement fall by hemorrhage class, as fractions of that body's own blood volume; death past the class IV band unless treated. A tourniquet on a limb stops its bleeding and starts an ischaemia clock (Proposal).
3. Each open wound has a bacterial infection hazard: base rate by type × contamination × (1 if not cleaned, lower if cleaned with clean water, lower still with disinfectant) × dressing condition. Onset shows after a delay (redness, heat, swelling, then fever).
4. Untreated infection progresses local → spreading → systemic (sepsis) on hazards; antibiotics are out of Tier 0, so cleaning early is the counter.
5. Fractures block weight-bearing (leg) or grip (arm) on that side until splinted, then reduce it.
6. Pain is a readout of wounds (less when the region is still); it affects only the capabilities whose regions hurt ([13](13-capabilities.md)).
7. A bite or scratch from a walker also records a pathogen dose ([21](21-infection.md)).
8. Wounds stay readable: the inspector shows region, type, depth and state, so a death can be explained.

## Invariants

- Blood volume ≥ 0; a wound belongs to one region.
- A cleaned wound never has a higher infection hazard than the same wound uncleaned.

## Tests

- Untreated deep laceration on the thigh: bleeds out inside the hemorrhage band; with a dressing, survives. (A3)
- 100 seeded untreated dirty lacerations: infection rate inside the band; cleaned ones lower. (A3)
- Splinted fracture walks faster than unsplinted. (A3)

## Calibration

| Value | Source |
| --- | --- |
| Blood volume | Nadler et al. (1962) |
| Region mass fractions | de Leva (1996) segment parameters, scaled per body |
| Tourniquet time limits | Kragh et al. (combat tourniquet studies); to check |
| Hemorrhage classes and effects | ATLS (American College of Surgeons), classes I–IV |
| Laceration infection rates (ED, cleaned) | Emergency medicine reviews (e.g. Hollander and Singer); higher for bites (dog and human bite literature) |
| Time to visible infection | Typical 24–72 h for wound cellulitis (clinical references; tune) |

## Model as implemented (stage 3)

- **Regions.** One static table: de Leva mass fractions and the prototype's authored vessel weights (neck 2.5, pelvis 2.0, head 1.8, chest 1.5, thigh 1.4, abdomen 1.3, shin 1.1, upper arm 1.0, forearm and foot 0.9, hand 0.8).
- **Wound ids.** Every wound has its own entity id, which keys its hazard draws, so adding a wound never shifts another wound's dice.
- **Bleeding.** Computed on the fine tick (0.5 s). Rate (mL/min) = depth base (skin 0.3, fat 1, muscle 6, deep 15) × vessel weight × length / 50 mm × (1 − clot), × 0.3 for a scratch, × 0.15 dressed, and 0 under a tourniquet on a limb.
- **Clotting.** Clots grow toward a cap with a time constant by depth (3, 8, 30, 120 min). Only deep wounds in regions with vessel weight ≥ 1.4 keep bleeding undressed: cap = 1 − 0.4 × min(1, length / 80 mm). A clot loses 20 % per minute while its region moves.
- **Blood loss.** Hemorrhage classes are fractions of the body's own volume; death at 45 % lost. Plasma refills from body water at 1.5 L/day while the deficit is under 8 %.
- **Infection onset.** Rate per day = base by type (scratch 0.02, laceration 0.041, puncture 0.06, bite 0.123) × contamination × cleaning factor (1, water 0.3, disinfectant 0.15) × 0.7 if dressed. The infection stays hidden for 36 h, then shows (event).
- **Infection course.** Each stage is an exit set per day of {resolve, worse}: local {0.4, 0.2}, spreading {0.2, 0.25}, systemic {0.05, sepsis death 0.5}; cleaning halves and a dressing cuts by 30 % the local stage's "worse" rate, so care after onset still helps. Fever is +1 °C spreading and +2 °C systemic. No healing while infected.
- **Healing.** Skin 5 days, fat 10, muscle 21, deep 35, fracture 42.
- **Pain.** Per wound: depth base (0.1 / 0.2 / 0.35 / 0.5); scratch 0.05; contusion 0.15; fracture 0.7 (0.3 splinted); a visible infection adds 0.15–0.4. This is scaled by age (0.5 + 0.5·e^(−days/3)) and by 0.6 when the region is still. Pains combine as 1 − Π(1 − p).

**Lab results:** an untreated deep 80 mm thigh laceration bleeds out in 1.4 h; dressed at once, the worst loss is 6 %. Of 100 dirty forearm lacerations over 7 days, 20 % are infected uncleaned, 6 % cleaned with water and 5 % disinfected.

## Not in Tier 0

Full layer-by-layer trauma geometry, surgery, burns, amputation, organs, antibiotics and drugs.
