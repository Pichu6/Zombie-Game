# Sensors and the adaptive soundscape

**Date:** 2026-10-10. **Status:** design draft, not a decision. It builds on [compute-budget](compute-budget.md), [engine architecture](engine-architecture.md) (audio reads the published snapshot, phase 8), [spec 18](../tier0/specs/18-noise-field.md), [spec 36](../tier0/specs/36-fields-and-weather.md) rule 6 (`ambient_db`), [0061](../decisions/0061-the-world-runs-without-the-player.md) (watching changes nothing), the open question in [threats](threats.md) on audio occlusion from the simulation's path loss (ES-46), and the research notes RA, XD, HST and PKC.

## The user's words (verbatim, lightly cleaned)

> "We need to have sensors or small scripts here and there for low-level scan and metrics, control. Is that a good idea? It could be that for sounds there's some sort of dB layer, from which sound could be adaptable: if it reads high dB it goes to loud city sounds, if it's low dB it goes to a more silent ambience. It should detect if the player is in the countryside in management view, or watching a country on the world map, or watching and managing a city, or panning around the world, passing different landscapes, biomes, situations, etc."

## 1. Sensors: yes, in four kinds, with one rule

**The rule:** a sensor may read anything, and only a governor may act, on things that never change what happens. A measured millisecond must never decide anything inside the simulation, or two PCs would play different worlds (XD-136).

| Kind | Reads | Writes | Deterministic? | Examples |
| --- | --- | --- | --- | --- |
| **Probes** (engine metrics) | Wall-clock time, counts | A debug overlay, a log file | No, and they never feed the simulation | Per-subsystem and per-chunk timers next to the per-tick hash (ranked item 1); hot agents, wakes per tick, path searches, sound searches; a frame profiler such as Tracy |
| **Watchers** (invariants) | Simulation state | Assertions, test failures | Yes | The ledger check already in the tick (spec 05 rule 1); two assertions per function (HST-112); `SIM_BUGGIFY` in test builds (HST-106) |
| **Readers** (summaries of the world) | The committed snapshot | Nothing in the simulation; they feed audio, UI, the attention list, the director | Yes, computed from state | Loudness per district (`ambient_db`), activity counts (fights, shots, traffic), beacon flags per region (XD-125), the attention list (PKC-160) |
| **Governors** (control) | Probes and readers | Only things that cannot change outcomes | Their output is outside the hash | The speed ladder and its caps (speed is not saved or hashed, spec 05 rule 3); dropping watched-but-not-played areas to warm where agreement is A1 (RA-115); refresh rate of pinned windows; render and audio detail |

Probes and governors cost little: a timer zone is tens of nanoseconds, against microseconds per agent (compute budget §1). Probes compile out of shipping builds or are switched off by default.

## 2. The soundscape: one dB layer, read differently by zoom

The dB layer the user describes already exists in the simulation:
- **Street level:** the noise field per cell (spec 18).
- **District and wider:** the ambient level from the sum of sources (spec 36 rule 6).

The audio should read **the same numbers the walkers hear**, so what the player hears matches what pulls the horde. That answers ES-46.

**A listener for every mode.** A listener is the point or area audio is computed for:

| Where the player is | Listener | What plays |
| --- | --- | --- |
| Embodied, street (Individual) | The pop's ears | Positioned sources with the simulation's own path loss through walls and doors; the ambient bed from the local level |
| Detached or managing a city (City layer) | The area under the camera, sampled at a few points weighted by screen area | Blended beds: city roar, suburb, quiet dead city, by the sampled dB and density; distant one-shots for real events (shots, sirens, a horde); weather on top |
| Region or country (management, RTS) | The region in view | Beds by biome and land use (countryside, forest, coast, mountains), weather and time of day; aggregated events as distant stingers (artillery over a front) |
| World map (board-game view) | None | **Silence, or music only** (the user's choice, 2026-10-10) |
| Panning across the world | Moves with the camera | Beds cross-fade as biomes, cities, weather and day and night pass under the view, at whatever gain the current height allows |

**Whose ears** (the user, 2026-10-10: "if I'm decoupled from a pop I hear where I zoom, I'm the camera; if I'm taking control of a pop, I hear where I am"):
- **Detached or observer:** the camera is the listener, and the zoom fade below applies.
- **Embodied:** the pop's ears are the listener, wherever the camera or the view goes.
  - **Example:** a president at his desk looking at the world map hears his office (the radio, the phones, the door), not the map.
  - This also keeps the body's danger audible: you hear the door being broken while you plan.
- **One listener at a time.** Pinned windows are silent, except a short alert cue when the pinned body or place is in danger.

**Zoom fades the world in** (the user, 2026-10-10: "hear nothing in world view, music perhaps, and as I'm zooming in I start hearing the ambience of where I'm zooming"). Ambience gain is a function of camera height on a log scale, using the zoom bands of [ZL-45](../research/zoom-and-layer-two.md):

| Band | Height | Ambience | Music |
| --- | --- | --- | --- |
| Globe | above about 2,000 km | 0 (silent) | full, if on |
| Region | 40–2,000 km | rises from 0 to about a third: weather and biome only | fading |
| City | 5–40 km | about two-thirds: city or countryside beds, distant events | low or off |
| District, street | under 5 km | full: positioned sources at street level | off, unless the player keeps it |

Rules for the fade:
- **Listen where the zoom is going.** While zooming, the listener is the zoom target (the point under the cursor when zooming with the wheel, or the selected place for Locate and Play as). While zooming in toward a city you hear that city, not the centre of the old view.
- **Zooming out reverses the fade** down to silence on the globe.
- **The curve and band edges are tuning values** in a data file, so the user can set how early the world starts to sound.

**Parameters**, computed each frame from cheap reads of the snapshot:
- local and ambient dB, grid alive or dead;
- biome and land use under the view, population and traffic density;
- weather (rain, wind, storm from the storm records);
- sun altitude (day, twilight, night);
- activity counts (fights, shots, fires, crowds);
- zoom height and layer.

These map onto an audio middleware's real-time parameters, which blend ambience beds and trigger one-shots.

**Smoothing.** Use two thresholds per switch, as a Schmitt trigger does (XD-113), with cross-fades of a few seconds, so panning across a border does not flicker the bed.

**Knowledge mode matters for audio too.**
- **Embodied:** you hear exactly what your pop hears. That is perception, so it is the truth at the pop.
- **Detached in knowledge mode:** remote soundscapes may use only what is public or known: biome, weather, time of day, and events your side knows of. Otherwise hearing gunfire over a city would tell you about a fight nobody reported.
- **Free (admin):** everything.

**What exists and what doesn't:**
- The simulation already has both dB layers and the sources.
- No audio plays yet. Sound marks in the window are the queued stand-in.
- miniaudio and Steam Audio are listed in the engine architecture.

## 3. Proposals (for the ranked list)

1. **Probes:** per-subsystem and per-chunk timers and counters, a debug overlay and a log, beside the per-tick hash (with ranked item 1).
2. **Readers as one module** fed from the published snapshot: district loudness, activity counts, beacons, attention list; consumed by audio, UI and the director.
3. **Governor rules written down:** what may be changed by measured load (speed caps, detail where A1, pinned-window refresh, render and audio detail) and what never may.
4. **Soundscape spec** when audio starts: listener per mode, parameters, beds by zoom band, smoothing, the knowledge-mode rule, audio reading the simulation's path loss (ES-46).
