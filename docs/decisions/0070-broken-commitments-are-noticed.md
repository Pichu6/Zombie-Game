# 0070 · Broken commitments are noticed: absence, search and consequences

- **Date:** 2026-10-10
- **Status:** Decided by the user as intent (2026-10-10). The mechanism and its numbers are defaults taken by Claude ([0052](0052-one-executable-one-simulation.md)), from research [absence-and-missing-persons](../research/absence-and-missing-persons.md) (AB). The user's words (lightly cleaned):

  > "If I take possession of a police officer, it takes him away from his schedule and leaves him in another part of the world. That should be perceived by the pop and by the pops who know him or are related to him. There must be consequences for a police officer skipping work for several days and turning up somewhere else: people asking about him and looking for him. How could that be handled?"
- **Builds on:**
  - [0061](0061-the-world-runs-without-the-player.md): watching changes nothing; possessing is an input;
  - [0062](0062-the-game-loop-attention-possession-and-late-starts.md);
  - [0068](0068-entering-a-world-the-home-pop-and-pins.md): the home pop;
  - [0069](0069-jobs-institutions-and-daily-lives-one-model-depth-bought-by-attention.md): schedules, institutions as queues;
  - [cases and living lore](../design/cases-and-living-lore.md) §14 (strange cases) and §15 (absence);
  - specs [24](../tier0/specs/24-pop-ai.md) and [39](../tier0/specs/39-orders-and-belief.md).

## Decision

1. **Every commitment creates expectations in others.** A shift start, arriving home or a meeting names who expects it and a tolerance. A kept expectation costs nothing. A broken one is an event.
2. **Those who expected him react by their own models, on one ladder:**
   1. phone him;
   2. ask around (work, friends, hospitals);
   3. report him missing (there is no waiting period);
   4. search.

   A phoned-in absence is ordinary and starts nothing ([AB-1](../research/absence-and-missing-persons.md)).
3. **Institutions handle the rest as cases** (0069 rule 3):
   - a missing-person case with a risk tier and a national record, so a check in another city is a hit;
   - an employer's absence rule (US default: separation after 3 consecutive scheduled days without contact, paused by a missing-person report);
   - for police employers, a conduct case after the return.
4. **On release the pop tries to get home,** and his account of the gap follows the strange-case dissonance (cases §14).
5. **Consequences persist:** a lost job, a strained household, a reputation, and in this world a suspicion of infection. Each traces back to the possession in the cause trace.
6. **Real people go missing without any player** (base rates per country: AB), so a possession hides among them.

## Consequences

- Schedules (the calendar slice) must carry their commitments and holders from the start.
- Data: `data/life/expectations.toml`, absence rates per occupation, `data/institutions/police/missing.toml`, absence policies per country (AB §3).
- Unsourced steps are defaults until sourced: how soon a supervisor calls, a home visit for a missing officer, the family's first hours.
