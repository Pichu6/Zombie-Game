# Pop lore, custom pops and player slots

**Status:** Proposal, except where [0040](../decisions/0040-pop-lore-custom-pops-and-player-slots.md) records the user's intent. **Needs:** [spec 28](../tier0/specs/28-pop-levels.md), [spec 29](../tier0/specs/29-embodied-pops.md), [layer jumps](layer-jumps.md), [content tools](content-tools.md).

## 1. The idea

A pop has a background (where from, family, work, debts, a past) and traits. These should give the world around the pop depth: the people, places and events in the pop's story exist in the simulation, so lore is not text pasted on top. The simulation adapts around the pop. The same holds for a pop the user authors.

## 2. The rule

> **Lore is filled in lazily and must agree with everything already committed. The world adapts only where nothing has been recorded.**

Committed state is the ledger, death and birth records, saved bodies, and anything a player has seen. It is never edited to make a story work. This keeps the simulation's own laws (conservation, one source of truth, random-access lives) intact.

## 3. Hooks

A background is a list of **hooks**: claims about the world, such as "her brother worked at the refinery", "he owes a debt to a rival", "they left the city in a famine year". A hook has a kind, a required shape (a pop of a certain age and sex, a building of a kind, an event in a year range) and a **resolution**:

| Resolution | When | Effect |
| --- | --- | --- |
| **Link** | An existing pop, place or event fits | Point at it. Nothing is created or changed. |
| **Fill** | Nothing fits, but the world has slack (an unrecorded lazy pop, a building with no recorded history) | Create the missing fact from a stateless keyed draw of (seed, pop, hook), written as a caused record. Replays identically. |
| **Open** | Neither works yet | Keep the hook unresolved. It resolves when the world grows a fit (a birth, a new tenant, an event), or never. |
| **Refuse** | The only way would break committed state | Drop the hook and log the reason. The author is told. |

Resolution is a function of the world and the hook only, so it does not depend on the order hooks are resolved in (agreement level A1; test: shuffle).

The other direction also holds: events the pop lives through (a plant closing, a bite, a famine) append to its background. The run's chronicle already does this.

## 4. Custom pops

A **custom pop** is an authored pop: name, age, sex, appearance, body spec, traits, background hooks, and optional fixed facts. It lives in a **pool** the user keeps across worlds (like XCOM's character pool) and is saved as data, not code.

**Entry paths** (all go through world commands, so they are replayable and carry a reason):

1. **From the start.** The pop is placed at world creation as a founder, in a place and household the setup chooses to fit its hooks (a pop who worked at a clinic starts near one).
2. **As a recruit, survivor or stranger.** The pop arrives during play through an in-world event (a survivor found, a refugee at the gate, a radio contact) at a time and place the simulation chooses, so it has a reason to be there. The 14-day run's found survivors and stranger already work this way ([stage 9](../tier0/plan.md)); a custom pop can fill that slot.
3. **Weighted mix.** The user sets whether pool pops are guaranteed or only drawn into candidate slots.

Constraints:

- A custom pop's id comes from the **founder slot** space of the invertible id scheme, so it is stable and unique.
- Its family and ties (hooks) resolve by §3, never by creating pops outside the registry's rules.
- Fixed facts that cannot fit the world are refused with a report, not forced.
- The sandbox option **no authored pops** disables the pool.

## 5. Player slots (multiplayer, and possession in single player)

A **slot** is a link: account, pop id. The pop stays in the registry at any level; the body comes from the pool ([spec 29](../tier0/specs/29-embodied-pops.md)).

- **Join:** embody the pop. Player slots are reserved in the pool and are never evicted for the AI's hot set.
- **Leave:** disembody after the dwell time and a safety rule (a logout timer in danger, or the body stays under AI for a while). Never an escape from a fight.
- **Offline time** must be applied (hunger, healing, wounds), so it needs the body `advance(from, to)` step, which spec 29 lists as its follow-up. Until it exists, multiplayer must not claim fairness.
- The same link serves switching pops, playing an officeholder ([0028](../decisions/0028-births-military-mystery-tech-leaders.md), [0029](../decisions/0029-leader-records-foxhole-tech-readout.md)) and spectating.
- Ownership: a pop belongs to at most one slot at a time.

## 6. Tests (planned)

- Hook resolution order independence (A1).
- No hook changes committed state: world hash of committed fields before and after resolution is equal.
- Fill is deterministic: same seed, same fill.
- A custom pop placed at the start and one recruited later both pass the registry audit; saves, loads and replays hash equal.
- Refused hooks leave the world unchanged and report a reason.
- A slot's pop cannot be evicted while linked; leave before dwell is refused.

## 7. Coherence across distance: what must already be true when you click

**Status:** Proposal (2026-10-10), from the user's fourth demo flow ([demo flow](demo-flow.md), fourth flow): a border fight, a robbery in a small town at night, the thief's background, the detective from the nearest city who is investigating him, the detective's marriage, his week and his desk. The user: "all that flow requires a level of procedural generation lore-wise, especially for it to have coherence with the world around it."

This is §2's rule applied to a whole session. Whoever the player clicks, anywhere, the answer must be fixed by the seed and the committed state. It must not depend on who asked first. The flow asks these questions:

| Question at the click | Where the answer comes from | Resolution (§3) |
| --- | --- | --- |
| Who fights whom at this border, and why? | The Macro conflict record and its cause trace ([spec 39](../tier0/specs/39-orders-and-belief.md)); forces sized from the country | Link to committed Macro state |
| Who are these soldiers? | Unit rosters of real pops ([spec 28](../tier0/specs/28-pop-levels.md)) with military lifepaths; roster counts equal Macro strength | Link to the roster; bodies lifted on approach ([EF](../research/coarse-layers-from-fine-rules.md)) |
| Why is this store open, and whose lights are those? | Opening hours by business kind and culture (data); homes lit by household schedules | A function of (time, establishment, household) |
| Why did he rob it? | An AI choice under pressure: a motive hook (debt, addiction, need) plus an opportunity | Simulated ([cases](cases-and-living-lore.md) §5); never placed |
| Has he done it before? | Criminal history as background hooks, drawn from the country's base rates and consistent with his record | Fill, keyed by (seed, pop, hook) |
| Is someone investigating him? | Each past crime that was reported opened a case at the police office with jurisdiction over the place; a keyed rule assigns it to one of that office's detectives | Link to the office and detective (police exist in the town generator's data); Fill the case record |
| Which police cover this town? | Jurisdiction from the settlement hierarchy | A function of the settlement map |
| What is the detective's private life? | Household, marriage bond, hooks (an affair: a secret tie to a real pop of fitting shape, with residue) | Link first, Fill if nothing fits; the affair has a cause in the simulation (long hours, stress) |
| Where is he now, and what will he do? | His occupation's weekly schedule, the calendar and his commute, plus deviations caused by events (a case, a secret meeting) | A function of (time, pop); deviations are recorded events |

**Rules:**
1. **Lazy, then committed.** An answer is computed when first needed (a click, a view, or the simulation itself) and is committed from then on ([0048](../decisions/0048-realistic-time-default-commit-on-observation-and-pins.md)). It never depends on who asked first.
2. **History is derived unless the world was run forward** ([0062](../decisions/0062-the-game-loop-attention-possession-and-late-starts.md) late starts). Derived history obeys base rates and committed records. A hook that cannot meet both stays open.
3. **Links before fills** (§3). An existing pop, place or record is always preferred, so stories knit into the world instead of adding strangers.
4. **Every derived fact leaves residue other views can find** (inhabitants, background residue; cases §6). Examples: the case file at the police office, the affair's traces, the thief's record.
5. **The knowledge level applies** ([0066](../decisions/0066-player-states-embodied-detached-observer.md)). In knowledge mode the thief's card shows only what the player's side could know, and the link to the case may stay hidden until it is found.

**From research (2026-10-10), replacing the first guesses** (an under-20,000 cut-off, 20–40 cases per detective, office hours for all):
- **Jurisdiction** ([SV-10, SV-11, SV-14 to SV-16](../research/services-by-settlement-size.md)). Each country names its policing model:
  - city departments (United States);
  - two national forces split by commune size (France, where the 20,000 line decides *which* force covers a commune, not whether it has detectives);
  - regional forces (England and Wales);
  - one national police, as the fallback.

  A case goes to the nearest office that covers the place at or above the level its kind needs (patrol, investigation, specialised unit) and has staff with the skill; ties go to the lower office id. About 15 % of officers investigate, so an office of fewer than about 7 officers has no detective. A small town gets the nearest city's detective by staffing arithmetic, not by a size line.
- **Staffing** ([SV-5, SV-19, SV-21](../research/services-by-settlement-size.md)). Officers per 1,000 residents follow the US size curve (2.3 at 1,000–9,999 residents, 1.6 at 50,000–99,999, 2.3 at a million or more), scaled by the country's police per 100,000.
- **Caseloads are measured, not set** ([IQ-11, IQ-23](../research/institutions-as-queues.md)). Open cases = arrival rate × mean time open (Little's law).
  - Arrivals pass a reporting funnel first (US 2024: 48 % of violent and 31 % of property victimisations reported). Unreported crime opens no case and stays hidden lore.
  - Times to outcome come from data (England and Wales medians: theft 8 days, robbery 35, sexual offences 65).
- **Schedules** ([TU-15, TU-41](../research/daily-lives-and-time-use.md)). Monday–Friday 08:00–17:00 is the most common office pattern, not the rule.
  - Each worker draws a pattern at hire. US shares: 85 % regular daytime, 5 % evenings, 3 % nights, the rest rotating, split or irregular; in hospitality, 40 % work outside daytime.
  - Start times follow the observed distribution: 57 % of US workers leave home between 06:00 and 08:29.
- **Criminal history** is drawn from country base rates through the same reporting funnel, never above them.

**Tests (added to §6):**
- **Click order:** resolve the nine answers in 100 random orders; results are identical.
- **Distance coherence:** the thief's case points to a detective whose office has jurisdiction over the place, and that detective's case list contains the thief.
- **Replay:** the same answers resolved during a replay match the live run.

## 8. Open

1. **How strong are authored facts?** Recommendation: hooks soft by default, user can mark a fact **fixed**, which makes the start or entry point adapt to it (place, time) but still never edits committed state.
2. **Content format** for the pool (TOML, like `data/gen/`).
3. **Spoilers:** mystery outbreak hooks stay hidden behind the reveal button ([0028](../decisions/0028-births-military-mystery-tech-leaders.md)).
4. **Multiplayer scope** is later; this doc only fixes the rules the code must not block.
