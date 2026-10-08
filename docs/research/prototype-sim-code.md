# Prototype simulation code: what to keep, what to avoid

**Date:** 2026-10-05. **Status:** research note, not a decision; reference only, none of this code is carried over, and everything below is a Proposal until a spec adopts it. **Feeds:** [body](../design/body.md), [body trauma](../design/body-trauma.md), [items and crafting](../design/items-crafting.md), [spec 05](../tier0/specs/05-clock.md), [spec 06](../tier0/specs/06-save-replay.md), [spec 11](../tier0/specs/11-body-composition.md), [spec 12](../tier0/specs/12-body-wounds.md), [spec 14](../tier0/specs/14-action-executor.md), [spec 16](../tier0/specs/16-view-2d.md), [spec 23](../tier0/specs/23-walker-brain.md). **Evidence:** read the Godot prototype's GDScript under `src/` as copied to the uploads folder: `biology/` (body system, body part, metabolism, hydration, moisture, cardiovascular, thermoregulation, environment, scent, clothing database, clock), `simulation/` (world state, inhabitant, body state codec, primitive packer, transfer actions, interaction offers, navigation, terrain parameters), `items/` (catalog, instance store), `entities/zombie_horde_manager.gd`, `ui/silhouette_paperdoll.gd` and `ui/body_foundations_lab.gd`; compared against [the Tier 0 plan](../tier0/plan.md) and specs 01–06, 08, 10–16, 18, 23 and 24. Most prototype numbers are authored (uncalibrated); literature values quoted against them were not re-read for this note and some are marked "to check". **IDs:** PC.

The user's guidance: use the old code as inspiration for systems, and keep state simple and compressed so deeper simulations fit later.

**Overall verdict.** The prototype's later code (world state, codec v2, transfers, offers) already moved toward our contract: one authority, validated loads, actions with reasons, no regenerated loot. The earlier body code is the opposite: about 40 floats per body updated every frame, many authored multipliers, and the same fact stored in two or three places. Keep the structure of the late code and the few sourced formulas; do not keep the body's numbers.

Labels used below: **sourced** (the code or a standard reference gives a source), **authored** (a number someone picked; uncalibrated), **wrong** (contradicts the literature our specs cite).

## Findings

Confidence `sourced` here means the prototype code was read; it does not make the code's own numbers right (most are authored).

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| PC-1 | Verdict: the prototype's late code (world state, codec v2, transfers, offers) already matches our contract (one authority, validated loads, actions with reasons); the early body code does not. Keep the late structure and the few sourced formulas, not the body's numbers. | estimate | this note |
| PC-2 | The prototype body stored about 40 floats per body and updated them every 0.1 s across seven subsystems; slow processes (fat, muscle, glycogen, hunger) should instead use spec 05's 60 s body step. | sourced | prototype `biology/` |
| PC-3 | Three exclusive mass compartments (fat, muscle, structural, kg) with total and fractions derived, saved as only the three masses in codec v2, worked in the prototype; spec 11 already does this. | sourced | prototype `biology/`, body state codec |
| PC-4 | Moving mass = body + worn gear + carried load (+ absorbed water) fed energy cost, heart load and impact, matching spec 11 rule 1 and spec 13's load factor. | sourced | prototype `biology/` |
| PC-5 | "The wall": below 25 % glycogen the stamina ceiling fell linearly from 100 to 20 (authored numbers); the real effect is exhaustion coinciding with muscle glycogen depletion (Bergström et al. 1967). | reported | prototype metabolism; Bergström et al. 1967 |
| PC-6 | Posture (stand, crouch, sit, lie) scaled recovery ×1.3 sitting and ×2 lying (authored); spec 10 uses a posture factor no spec defines. | sourced | prototype `biology/` |
| PC-7 | Sourced formulas in the code for later weather work: DuBois body surface area; body heat capacity 3.47 kJ/(kg·°C); 1 clo = 0.155 m²·K/W; 1 mL of sweat evaporated removes ~2.43 kJ; NWS wind chill; NWS (Rothfusz) heat index; sweat capped at ~42 mL/min. | reported | prototype thermoregulation |
| PC-8 | Wet clothing losing up to 75 % of its insulation is an authored number in the prototype. | sourced | prototype moisture/clothing |
| PC-9 | Katch–McArdle BMR = 370 + 21.6 × lean kg reads lean mass directly; an alternative to spec 11's Mifflin–St Jeor. | reported | prototype metabolism |
| PC-10 | Wrong numbers in the prototype: muscle loss at 4000 kcal/kg (Hall 2008 gives lean ~1.8 kcal/g, fat ~9.4 kcal/g); gastric emptying 15 kcal/min (literature closer to 2–4 kcal/min, to check); a fixed 85/15 fat/muscle split (spec 11 uses Forbes). | reported | prototype metabolism; Hall 2008 |
| PC-11 | The prototype's water model used a 2.5 L pool with death at 150 mL left (~3 % of body mass, several times too early) and baseline loss 0.02 mL/s (1.7 L/day). Spec 11's deficit as % of body mass is right but needs a lethal band. | sourced | prototype hydration |
| PC-12 | Stored readouts (hunger % on its own clock at 0.0037 %/s, dehydration level, shock, BP, max stamina, daily calories) drifted from their sources; derive them (spec 11 rule 7, spec 13). | sourced | prototype `biology/` |
| PC-13 | Heart rate was a target BPM summed from seven authored terms (mass load 0.42 BPM/kg, pain +28, shock +55, drift ≤ 22) and smoothed; if HR ever appears it should be derived. | sourced | prototype cardiovascular |
| PC-14 | Death was a list of hard-coded thresholds in `update` that also emitted UI strings; causes belong to events (spec 25). | sourced | prototype `biology/` |
| PC-15 | Body parts used 17 regions (same split as spec 12) with a static table of parent/children and vessel weight: neck 2.5, pelvis 2.0, head 1.8, chest 1.5, thigh 1.4, shin 1.1, upper arm 1.0, forearm and foot 0.9, hand 0.8 (authored). | sourced | prototype body part |
| PC-16 | Wound base bleed rates (authored): scratch 0.1, laceration 0.8, bite 1.2, deep cut 2.2, arterial 4.5 mL/s, scaled by (BPM/75)^1.5. | sourced | prototype body part |
| PC-17 | Treatments were separate states per wound (pressure/dressing, splint, disinfected, tourniquet); the tourniquet is missing from spec 12. | sourced | prototype body part |
| PC-18 | Pain from use: foot and leg strain from impact scaled by (moving mass/70)^1.8, ×3.5 barefoot, crouching > 3 min, standing > 45 min (authored); a later source of "why is she slow". | sourced | prototype body part |
| PC-19 | Body-part errors to avoid: one wound slot per region (a new wound overwrote the old), stored accumulated pain with per-cause clamps, instant infection (+45 % on a bite), fixed 5000 mL blood and shock thresholds 4000/3500/2800/2000 mL, fixed region masses summing to 78.8 kg, amputation bleed of 120 mL/s, and "bandage stops all bleeding". | sourced | prototype body part |
| PC-20 | The prototype clock used float seconds × speed, Day 1 at 08:00, 30-day months, a 0.1 s step with a 1e-9 epsilon, speed capped at 60× and at most 60 simulated s per call. Keep the fixed step and bounded work per call; avoid float time and speed as a multiplier on delta (spec 05). | sourced | prototype clock, world state |
| PC-21 | World state was the only authority, with versioned migrations (v1→v2→v3), load into a candidate world committed only if valid, save to a temp file then re-validate and rename, and floats stored bit-exact. Keep all of these (spec 06). | sourced | prototype world state, codec |
| PC-22 | Save-format errors: a reflection-driven schema saved every script variable (caches became save format, refactors broke saves); JSON with tagged hex floats cost roughly 4–5× raw binary; string ids needed parsing; garment copies per region needed alias restoration. | sourced | prototype codec |
| PC-23 | Interaction offers carried a label, enabled flag and reason; execution rebuilt the list ("a rendered menu is never authorization"); offer ids encoded origin and destination so a stale menu could not act on a moved object. | sourced | prototype interaction offers |
| PC-24 | Transfers kept the item at its origin, reserved, until commit; manipulation started the step after arrival ("travel is never free work"); navigation was a deterministic BFS on a 0.5 m grid with a fixed 4-neighbour order, and a door could not close on a body in it. | sourced | prototype transfer actions, navigation |
| PC-25 | Transfer gaps: only between containers reachable from one standing point (1.5 m), and authored duration (access + 0.5 + 0.5 s/kg, × (0.3 + 0.7·stamina) × (1 − 0.65·pain)); hauling across rooms needs the item in the actor's hands, which spec 14 does not yet define. | sourced | prototype transfer actions |
| PC-26 | Items: shared definitions vs instances with Q 0–100 and condition 0–1, flat containers, and a "still at origin?" check on transfer; ~40 garments with clo, mass, water uptake, wind, waterproofing, ballistic and cut protection, layer and covered regions (authored, some cross-checked against CDDA, which may not be copied: CC BY-SA). | sourced | prototype `items/`, clothing database |
| PC-27 | Item errors: every unit an instance (fungibles should be stacks, spec 03), no nesting (backpack contents as a separate scalar), sim and view data in one record, and per-region clo summed and divided by 13 when there are 17 regions. Cut protection took the max over layers. | sourced | prototype `items/` |
| PC-28 | The horde used boids for every zombie every physics frame (separation 2.5, alignment 1.0, cohesion 1.2, target 3.0, radius 6 m, top speed 3.2 m/s, authored); sounds attracted within `dB × 1.5` metres for 3 s. Avoid O(n²) per frame, dB as metres and a global target. | sourced | prototype `zombie_horde_manager.gd` |
| PC-29 | The paperdoll drew 17 clickable regions, one status colour per region by fixed priority (severed > bleeding > tourniquet > dressed > fracture > pain > 25) plus markers (pulsing rings faster above 2 mL/s bleed); keep it as the Body tab of the inspector (spec 16). | sourced | prototype `silhouette_paperdoll.gd` |
| PC-30 | A one-line "sensory report" naming the single most urgent body issue is a cheap answer to "can the player tell why?". | estimate | this note |
| PC-31 | Compression: save stocks and derive readouts; hold bounded state (S, reserve, fear, morale, contamination) as `uint16` fractions; use µL, J and mg in `int64` for slow flows; static per-region tables in data; dense sorted `uint32` ids; stacks for fungibles; binary saves in fixed order; walkers off-site as census counts. | estimate | this note |
| PC-32 | A packed wound record fits in about 20 bytes (region 5 bits, type 3, depth 2, flags 3, infection stage 3, length `uint16` mm, contamination and clot `uint8`, dose `uint16`, created tick `int32`, dressing id `uint32`); the bleed rate is derived, not stored. | estimate | this note |
| PC-33 | A 0.1 mL/s scratch bleed is 0.05 mL per 0.5 s tick, which rounds to zero forever in whole mL; baseline water loss is ~1.2 mL per 60 s step. So spec 01 needs a saved remainder per flow or finer units. | derived | this note (from PC-16, PC-11) |
| PC-34 | Spec changes suggested (not applied): body water as its own compartment (spec 11); a lethal dehydration band with a source (Adolph 1947, to check); the wall; posture; hemorrhage classes as fractions of the body's own blood volume and derived pain (spec 12); de Leva (1996) region mass fractions; tourniquet with ischaemia after ~2 h (Kragh et al. 2009, to check); item location per action phase, carried in hands, and "arriving is not working" (spec 14); candidate load and temp-file save (spec 06); garment protection combination rule (spec 15); a 17-region Body tab (spec 16). | estimate | this note |

## 1. Body: metabolism, hydration, cardio, thermo

**What it did.** `BodySystem.update(delta)` ran every 0.1 s and called seven subsystems in a fixed order. It stored three tissue masses (fat, muscle, structural, in kg), glycogen as %, stamina 0–100, hunger %, a 2.5 L water pool, 5000 mL blood, heart rate, blood pressure, core and skin temperature, sweat and wetness, scent, posture and timers. It also wrote a first-person "sensory report" for the UI.

**Worth keeping**
- **Three exclusive mass compartments, with total and fractions derived** (`total_weight_kg` and `_pct` are getters, and codec v2 saves only the three masses). Spec 11 already does this; the prototype shows that it works and that legacy "shape coordinates" had to be migrated away from.
- **Moving mass = body + worn gear + carried load (+ absorbed water)**, which feeds energy cost, heart load and impact. It matches spec 11 rule 1 and spec 13's load factor.
- **"The wall":** below 25 % glycogen, the stamina ceiling falls linearly from 100 to 20 (authored numbers, real effect: exhaustion coincides with muscle glycogen depletion, Bergström et al. 1967). Spec 11 lacks this link (see the spec changes below).
- **Posture** (stand, crouch, sit, lie), with recovery ×1.3 sitting and ×2 lying (authored). Spec 10 needs a posture factor that no spec defines.
- **Sourced formulas for later (weather is out of Tier 0):** DuBois body surface area; body heat capacity 3.47 kJ/(kg·°C); 1 clo = 0.155 m²·K/W; evaporating 1 mL of sweat removes ~2.43 kJ; NWS/Environment Canada wind chill; the NWS (Rothfusz) heat index; sweat capped at ~42 mL/min. Wet clothing losing up to 75 % of its insulation is authored.
- Katch–McArdle BMR (370 + 21.6 × lean kg) reads lean mass directly, which suits our composition state. Spec 11's Mifflin–St Jeor also works; this is noted only as an alternative.

**Avoid**
- **Per-frame integration of slow processes.** Fat, muscle, glycogen, hunger and conditioning all changed every 0.1 s. Spec 05's 60 s body step is right; keep it.
- **Stored readouts.** `hunger_pct` grew on its own clock (0.0037 %/s, authored) next to the real stomach store; `dehydration_level`, `shock_state`, BP, `max_stamina`, `calories_burned_today` and `is_heat_exhausted` were all saved. Each one can drift from its source. Spec 11 rule 7 and spec 13 are right to derive them.
- **Wrong numbers:** muscle loss at 4000 kcal/kg (Hall 2008 gives lean tissue ~1.8 kcal/g and fat ~9.4 kcal/g); gastric emptying at 15 kcal/min (closer to 2–4 kcal/min in the literature, to check); a fixed 85/15 fat/muscle split (spec 11 uses Forbes).
- **The water model.** It used a 2.5 L "functional pool" with death at 150 mL left, about 3 % of body mass, which is several times too early. Baseline loss was 0.02 mL/s (1.7 L/day, while the comment says 1.5). Spec 11 is right to use the deficit as % of body mass; it still needs a lethal band.
- **The heart-rate stack:** a target BPM summed from seven authored terms (mass load 0.42 BPM/kg, pain +28, shock +55, drift ≤ 22…) and smoothed with `exp`. Spec 11 has no heart rate, and it should stay derived if it ever appears (design/body.md lists HR as a state number; Tier 0 does not need it).
- **Magic multipliers everywhere:** `fitness_ratio = (muscle%/(fat%+0.1)) × (conditioning/40)` clamped 0.35–2.8; three exertion bands (0.02/0.35/0.75) with step changes in cost. Spec 11's MET per action plus Pandolf replaces all of it.
- **Death as a list of hard-coded thresholds** inside `update` that also emitted UI strings. Causes belong to events ([25](../tier0/specs/25-chronicle.md)).

**Informs:** [11](../tier0/specs/11-body-composition.md), [13](../tier0/specs/13-capabilities.md), [05](../tier0/specs/05-clock.md).

## 2. Body parts and wounds

**What it did.** It had 17 regions, the same split as spec 12. Each `BodyPart` held integrity, a scalar `bleeding_rate_base`, pain 0–100, flags (fracture, bandaged, splinted, disinfected, tourniquet, severed), **one** active wound type, `infection_level`, a vessel weight, its parent and children (for amputation cascades), sockets, rig bone names, and full copies of every worn garment.

**Worth keeping**
- **Static per-region table:** parent region and child regions, vessel weight (neck 2.5, pelvis 2.0, head 1.8, chest 1.5, thigh 1.4, shin 1.1, upper arm 1.0, forearm and foot 0.9, hand 0.8; all authored). It is one data table, not per-body state.
- **Wound type ladder with base bleed rates** as a first guess to replace: scratch 0.1, laceration 0.8, bite 1.2, deep cut 2.2, arterial 4.5 mL/s (authored). Bleed scaled by (BPM/75)^1.5 (authored); in our terms, bleed rises with exertion, which spec 12 rule 1 covers as "movement".
- **Treatments as separate states per wound:** pressure/dressing, splint, disinfected, tourniquet. The tourniquet is the one spec 12 lacks.
- **Pain from use, not only from wounds:** foot and leg strain from impact (scaled by (moving mass/70)^1.8, ×3.5 barefoot, authored), crouching more than 3 min, standing more than 45 min. This is out of Tier 0, but a good later source of "why is she slow".

**Avoid**
- **One wound slot per region.** A new wound overwrote the type, reset bandage and disinfection for the whole part, and added to a shared bleed scalar. Spec 12's wound list is right.
- **Pain as a stored, accumulated number** with clamps per cause (30, 35, 40, 45, 50). It never matched the wounds it came from.
- **Instant infection** (a bite adds +45 % infection). Spec 12 rules 3–4 use hazards with an onset delay.
- **Fixed blood and region masses:** 5000 mL of blood and shock thresholds at 4000/3500/2800/2000 mL, whatever the body size; region masses fixed in kg (they sum to 78.8 kg for every body). These should be fractions of the body's own volume and mass.
- **An amputation bleed of 120 mL/s** (more than cardiac output) and "bandage stops all bleeding" (wrong in both directions).

**Informs:** [12](../tier0/specs/12-body-wounds.md), [13](../tier0/specs/13-capabilities.md), [20](../tier0/specs/20-damage.md).

## 3. Clock

**What it did.** `SimulationClock` accumulated float seconds × speed, started on Day 1 at 08:00, and used a 30-day month and a 360-day year. `WorldState.advance` added float real time × speed into an accumulator, stepped every 0.1 s with a 1e-9 epsilon, capped speed at 60× and refused more than 60 simulated seconds per call.

**Keep:** a fixed step decoupled from frame rate; a bounded amount of work per call (offline catch-up is chunked on purpose); the start hour as data.
**Avoid:** float time and epsilon comparisons (spec 05's `int64` tick is right); calendar months (Tier 0 is 14 days, so day and hour are derived from the tick); speed as a multiplier on `delta`, which changes what a step does (spec 05 rule 3 forbids this).

**Informs:** [05](../tier0/specs/05-clock.md).

## 4. World state, save and load

**What it did.** `WorldState` was the only authority ("views never tick bodies"). It owned actors, inventory, actions and navigation, with a schema version and explicit v1→v2→v3 migrations. `restore` decoded everything into a **candidate** world and committed only if every check passed. `save_file` serialised, re-validated the result by loading it, wrote to `path.tmp` and renamed it. Body floats were stored bit-exact as 16-digit hex strings.

**Keep:** a single authority; versioned upgrades with a test per old version; validate into a candidate, then swap (a failed load never touches the live world); save to a temp file, validate, then rename; exact float bits in saves (the prototype's instinct was right; spec 06's byte-identical round trip needs it).
**Avoid:**
- **A reflection-driven schema:** the codec saved every script variable, so caches and readouts (HR, BP, hunger, dehydration, daily calorie counters, display text inputs) became save format, and every refactor broke saves.
- **JSON with tagged values:** floats as hex strings inside `{"kind","value"}` objects cost roughly 4–5× the bytes of raw binary.
- **String identities** ("inhabitant-3", "item-12") that had to be parsed back and range-checked.
- **Garment dictionaries copied into each region**, then "alias restoration" on load to make them one object again.

**Informs:** [06](../tier0/specs/06-save-replay.md), [01](../tier0/specs/01-quantity.md), [03](../tier0/specs/03-stock-transfer.md).

## 5. Transfers, offers and navigation

**What it did.** Objects declared `interaction_ids`. `InteractionOffers.query` built offers (label, enabled, **reason**, phases, effect text). `execute` rebuilt the list against the current state ("a rendered menu is never authorization"). Offer ids encoded **origin and destination**, so an old menu could not act on an object that had moved. `TransferActions` was a timed action with two phases (approach, then manipulate). The item stayed at its origin, reserved, until commit; an interruption left it there. Arriving started manipulation on the *next* step ("travel is never free work"). Navigation was a deterministic BFS on a 0.5 m grid with a fixed 4-neighbour order and segment-versus-box clearance; closed doors were obstacles, and a door could not close on a body standing in it.

**Keep:** offer = data plus reason, and execution re-checks (spec 14 rules 1–2); offer identity tied to the target's location; one reservation per item; one action per actor; the item in exactly one place at every phase; a fixed neighbour order (spec 05 rule 7); "can't close a door on a body"; and the rule that arrival does not count as work.
**Avoid:**
- Transfers only between two containers reachable from **one** standing point (1.5 m). Hauling across rooms needs the item to be in the actor's hands while walking, which spec 14 does not define yet.
- Authored duration: access s(origin) + access s(dest) + 0.5 + 0.5 s/kg, with rate × (0.3 + 0.7·stamina) × (1 − 0.65·pain). Spec 14 reads work rate from 13.
- Effort 0.25/0.15 as a fake exertion override written onto the body each step.
- A continuous-geometry path saved as float points; recomputing actor carried mass by scanning every item every step.

**Informs:** [14](../tier0/specs/14-action-executor.md), [09](../tier0/specs/09-site-map.md), [10](../tier0/specs/10-fortification.md).

## 6. Items

**What it did.** `ItemInstanceStore` had shared definitions (id, mass kg, interaction ids), flat containers (owner, capacity kg, −1 = unbounded) and instances (definition, container, Q 0–100, condition 0–1). `transfer` checked that the item was still at the expected origin. `ItemCatalog` merged simulation data with visual metadata (icon, model, asset status, components). `ClothingDatabase` had ~40 garments with clo, dry mass, water uptake, wind, waterproofing, ballistic and cut protection, layer and covered regions.

**Keep:** definition versus instance; Q and condition per instance (spec 08); an optimistic check on transfer ("still at origin?"); clothing data fields as a starting table for later weather work (all authored, some cross-checked against CDDA, which may not be copied: CC BY-SA). Cut protection per region took the **max** over layers, which is simple and conservative.
**Avoid:** every unit as an instance (nails and fuel should be stack stocks, spec 03); no nesting (a backpack's contents were a separate scalar `backpack_content_weight_kg`, which duplicated state); simulation and view data in one record (keep two files: sim definitions in `data/tier0/`, view metadata beside the view); summing per-region clo and dividing by 13 when there are 17 regions.

**Informs:** [08](../tier0/specs/08-assembly.md), [15](../tier0/specs/15-items-recipes.md), [03](../tier0/specs/03-stock-transfer.md).

## 7. Horde

**What it did.** It used boids for every zombie, every physics frame: separation 2.5, alignment 1.0, cohesion 1.2, target 3.0, radius 6 m, top speed 3.2 m/s (all authored). Sound stimuli were attractors with a range of `loudness_dB × 1.5` metres, lasting 3 s.

**Keep:** the stimulus memory that fades (spec 23 rule 2); following as local alignment and cohesion (spec 23 rule 7, herding).
**Avoid:** O(n²) per frame over every member; decibels used as metres (spec 18 propagates dB properly); a global "primary target" that all zombies know (spec 23: a walker knows only what its senses gave it); speed as a constant instead of from the body (spec 23 rule 8).

**Informs:** [23](../tier0/specs/23-walker-brain.md), [18](../tier0/specs/18-noise-field.md), [22](../tier0/specs/22-census.md).

## 8. 2D paperdoll and body view

**What it did.** `SilhouettePaperdoll` drew 17 clickable region polygons over a body image. Picking used the polygon first, then the nearest centroid, gated by a picking texture. Each region got **one status colour by fixed priority**: severed > bleeding > tourniquet > dressed > fracture > pain above 25. Markers sat at the region centroid: pulsing rings for bleeding (faster when above 2 mL/s), a green aura for infection, a zigzag for fracture, and icons for dressing, tourniquet and splint. There was also a backpack badge. Layer modes (muscle, skeleton, vessels, nerves) and 3D hologram morphing came on top. `body_foundations_lab` let the user edit the three masses and height, rejected invalid input atomically, and saved and loaded through the codec.

**Keep:** the region diagram as the Body tab of the inspector (spec 16); one colour per region by a fixed priority plus small markers; click a region to list its wounds; visual smoothing (lerp) only in the view; a lab that edits only the stored masses and shows the derived values. The sensory report, a priority chain that picks the single most urgent body issue and writes one line, is a cheap way to answer "can the player tell why?".
**Avoid:** the view reading many raw floats and re-deriving bleed rates itself (it should read [13](../tier0/specs/13-capabilities.md) readouts and wound records); anatomical layer modes, holograms and morph weights in Tier 0; display smoothing that ever writes back to the simulation.

**Informs:** [16](../tier0/specs/16-view-2d.md), [25](../tier0/specs/25-chronicle.md).

## Compression ideas

These keep the Tier 0 state small and every saved value a primary fact.

1. **Save stocks, derive readouts.** This is codec v2's lesson, applied to the whole body. Stored: fat, muscle, structure and water (each one once), glycogen, stomach (g and kJ), blood, S, exertion reserve, VO₂max, training stimulus (3), core temperature, fear, morale, posture, wound list. Never stored: totals, fractions, needs, hunger, pain, shock class, capabilities, speeds, HR. A debug check recomputes any cache and compares.
2. **Fixed-point for bounded state.** Hold S, exertion reserve, fear, morale and contamination as `uint16` fractions (1/65535) in the live state, not only in the save. Then save, load and replay are exact without storing doubles, and arithmetic is integer (spec 01 rule 2 applies).
3. **Finer integer units for slow body flows,** or a stored remainder per flow (see the spec changes). Blood and body water in µL, energy in J and glycogen in mg all fit in `int64` with room to spare.
4. **Static tables in data, not per body.** Region names, parent and children, vessel weight, segment mass fraction and tissue depth profile go in one table indexed by a 5-bit region id. Each body stores only what differs.
5. **Packed wound records.** Each wound needs region (5 bits), type (3), depth layer (2), flags (cleaned, splinted, tourniquet: 3), infection stage (3), length (mm, `uint16`), contamination (`uint8`), clot progress (`uint8`), pathogen dose (`uint16`), created tick (`int32` relative to the run), dressing instance id (`uint32`): about 20 bytes. The bleed rate is **derived** from type, depth, region, clot, dressing and the movement of that region, so it is not stored.
6. **Coverage as bitmasks.** A garment definition holds a 17-bit region mask and one protection value. A body stores the ids of what it wears; per-region protection is derived on change and cached.
7. **Dense ids and sorted arrays.** Use `uint32` indices for bodies, instances and containers, kept sorted (spec 05 rule 7), not string ids or hash maps. Location is one `uint32` per instance (the location tree of spec 03).
8. **Stacks for fungibles.** Nails, planks of one kind, fuel, water and food portions are integer stocks in a container, not instances. Instances are only for things with Q, condition or slots.
9. **Event-driven updates where a rate is constant.** A sleeping or resting body gets the same 60 s step, but bleeding, spoilage and water loss can schedule "time to the next threshold" as an event, like spec 18 rule 3 for continuous sounds. This keeps rest-until-event fast with ~60 walkers.
10. **Batched body ledger entries.** One entry per body per slow step per conversion pair (food→glycogen, glycogen→spent, fat→spent), not one per sub-step. The per-tick ledger check can run on a ring buffer of the current tick only.
11. **Binary save in a fixed order.** Little-endian, versioned, ints as ints, any remaining doubles as raw bits. JSON exports exist only for debugging.
12. **Walkers out of view as census counts** (spec 22), not as boids or full bodies; a walker gets a full body only when it enters the site.

## Spec changes suggested

Only where the prototype shows a gap or an error. Not applied; for the user to confirm.

1. **[01-quantity.md](../tier0/specs/01-quantity.md):** add a rule that a continuous rate flowing into an integer stock carries a **saved remainder per flow**, or that body fluids and energy use µL, J and mg. Reason: the prototype's scratch bleeds 0.1 mL/s, which is 0.05 mL per 0.5 s tick and rounds to zero forever in mL; baseline water loss is ~1.2 mL per 60 s step.
2. **[11-body-composition.md](../tier0/specs/11-body-composition.md), State:** make body water its own compartment (total = fat + muscle + structure + water) instead of "part of structure mass". As written, one gram is owned by two stocks, which is the double counting the prototype's v2 comment warns about.
3. **11, Rules and Tests:** add a lethal dehydration band as % of body mass with a source (e.g. Adolph 1947, to check). The prototype killed at ~3 %.
4. **11, Rules:** the exertion reserve ceiling falls as glycogen runs low ("the wall"; Bergström et al. 1967). Add a test: a long haul on an empty stomach tires sooner than a fed one.
5. **11, State (or 13):** add **posture** (stand, crouch, sit, lie), set by actions. Lying is needed to sleep; posture changes recovery, footstep noise, visibility and push force. [10](../tier0/specs/10-fortification.md) rule 1 already uses a "posture factor" that nothing defines.
6. **[12-body-wounds.md](../tier0/specs/12-body-wounds.md), Rules 2 and 6:** state hemorrhage classes as fractions of the body's own normal blood volume (ATLS uses %); say that pain is derived from each wound's type, depth, age and the movement of its region, so "lowered by rest" means less movement, not a stored pain value that decays.
7. **12, Calibration:** add region mass fractions from de Leva (1996), scaled by the body, instead of fixed kg per region.
8. **12, Rules (optional):** a tourniquet for limb regions stops bleeding below it, at the cost of ischaemia after about 2 h (tourniquet literature, e.g. Kragh et al. 2009, to check). If accepted, add "improvised tourniquet: cloth + stick" to [15](../tier0/specs/15-items-recipes.md).
9. **[14-action-executor.md](../tier0/specs/14-action-executor.md), Rule 6:** say where the item is in each phase. It stays at the origin, reserved, until picked up; then it is in the actor's **hands** (a container limited by the carry limit from 13); it is placed on "fit". An interruption while carrying drops it on the actor's cell. Also add that an offer's identity includes the target's current location, so a stale menu fails instead of acting.
10. **14, Rules:** add "arriving is not working". The work phase starts on the tick after approach ends, so travel never counts as progress.
11. **[06-save-replay.md](../tier0/specs/06-save-replay.md), Rules:** add "load into a candidate and swap only if valid (a failed load never changes the live world)" and "write to a temp file, verify by loading, then rename". Add "only primary state is saved; derived values are rebuilt on load".
12. **[15-items-recipes.md](../tier0/specs/15-items-recipes.md), Worn clothes:** say how two garments on one region combine (max, as in the prototype, or 1 − Π(1 − pᵢ)). Store coverage as a region mask in the definition.
13. **[16-view-2d.md](../tier0/specs/16-view-2d.md), Inspector:** the Body tab is a 17-region silhouette with one colour per region by fixed priority (bleeding > tourniquet > dressed > fracture > pain), markers per wound state, a click-to-list of wounds, and one "how I feel" line naming the top limiting capability.
