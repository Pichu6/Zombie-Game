# 12 · Body: regions and wounds

**Status:** Proposal. **Stage:** 3. **Needs:** [11](11-body-composition.md).

## Purpose

17 body regions with tissue layers, a wound list per region, bleeding and bacterial infection. This carries "a scratch that gets infected", "you bled out" and the bite that hides an infection.

## State

| Field | Unit | Notes |
| --- | --- | --- |
| regions | 17 | head, neck, chest, abdomen, pelvis; upper arm, forearm, hand, thigh, shin, foot × left/right |
| tissue per region | skin, fat (mm, from fat kg and distribution), muscle (mm), bone integrity | Fat and muscle share derived from [11](11-body-composition.md) |
| wound | type, region, depth layer, length (mm), bleed rate (mL/min), contamination (0–1), cleaned, dressing (instance with Q and condition), infection stage, pathogen dose | Many per region |
| blood volume | mL | ~70 mL/kg lean-adjusted; part of structure mass |
| pain | 0–1 per region and total | Derived |

Wound types in Tier 0: scratch, laceration, puncture, bite, blunt contusion, fracture.

## Rules

1. Bleed rate is set by depth and region (vessels), falls as a clot forms, rises again with movement of that region, and drops with pressure or a dressing.
2. Blood loss moves blood to a sink; consciousness and movement fall by hemorrhage class; death past the class IV band unless treated.
3. Each open wound has a bacterial infection hazard: base rate by type × contamination × (1 if not cleaned, lower if cleaned with clean water, lower still with disinfectant) × dressing condition. Onset shows after a delay (redness, heat, swelling, then fever).
4. Untreated infection progresses local → spreading → systemic (sepsis) on hazards; antibiotics are out of Tier 0, so cleaning early is the counter.
5. Fractures block weight-bearing (leg) or grip (arm) on that side until splinted, then reduce it.
6. Pain is a readout of wounds, lowered by rest; it affects only the capabilities whose regions hurt ([13](13-capabilities.md)).
7. A bite or scratch from a walker also records a pathogen dose ([21](21-infection.md)).
8. Wounds stay readable: the inspector shows region, type, depth and state, so a death can be explained.

## Invariants

- Blood volume ≥ 0; a wound belongs to one region.
- A cleaned wound never has a higher infection hazard than the same wound uncleaned.

## Tests

- Untreated deep laceration on the thigh: bleeds out inside the hemorrhage band; with a dressing, survives. (A3)
- 100 seeded untreated dirty lacerations: infection rate inside the band; cleaned ones lower. (A2)
- Splinted fracture walks faster than unsplinted. (A3)

## Calibration

| Value | Source |
| --- | --- |
| Blood volume | Nadler et al. (1962) |
| Hemorrhage classes and effects | ATLS (American College of Surgeons), classes I–IV |
| Laceration infection rates (ED, cleaned) | Emergency medicine reviews (e.g. Hollander and Singer); higher for bites (dog and human bite literature) |
| Time to visible infection | Typical 24–72 h for wound cellulitis (clinical references; tune) |

## Not in Tier 0

Full layer-by-layer trauma geometry, surgery, burns, amputation, organs, antibiotics and drugs.
