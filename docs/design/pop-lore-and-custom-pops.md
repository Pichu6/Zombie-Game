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

## 7. Open

1. **How strong are authored facts?** Recommendation: hooks soft by default, user can mark a fact **fixed**, which makes the start or entry point adapt to it (place, time) but still never edits committed state.
2. **Content format** for the pool (TOML, like `data/gen/`).
3. **Spoilers:** mystery outbreak hooks stay hidden behind the reveal button ([0028](../decisions/0028-births-military-mystery-tech-leaders.md)).
4. **Multiplayer scope** is later; this doc only fixes the rules the code must not block.
