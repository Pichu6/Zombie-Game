# Haven and Hearth: a reference game for long real-time play

**Date:** 2026-10-07. **Status:** research note, not a decision. **Why:** the user named it as the model for our long-wait style: real durations, progress-bar activities, a speed ladder ([0042](../decisions/0042-real-durations-played-as-progress-bars-with-a-speed-ladder.md), [0043](../decisions/0043-start-speed-and-time-while-closed.md)). Kept here as the "reference game" memory for it.

**Evidence caveat (as in the other notes):** the Ring of Brodgar wiki (the game's community wiki) and several news pages were **blocked by the proxy**, so everything below is from **web-search summaries** of those pages, not full reads. Numbers are "reported". Items marked *derived* are our arithmetic from reported figures. Re-check the primary page before relying on one. What I could not find is listed at the end.

## What it is

| Fact | Detail | Confidence |
| --- | --- | --- |
| Genre | Persistent-world multiplayer survival and crafting game ("MMORPG"), low-fantasy, Germanic-myth flavour | Reported, several sources |
| Makers and tech | Two developers, Jorb and Loftar (the team is called Seatribe); written in Java, C and Paintshop Pro (the wiki's "About" page) | Reported |
| Client and server | The client (hafen-client) is public, reported as LGPL-3 with parts under other licences; the **server is closed**. Read-only git at `sh.seatribe.se`; built with Apache Ant; mirrored on GitHub | Reported; the licence should be checked in the repo |
| History | Moved to 3D graphics in August 2015 with a new server; later an "eternal alpha" relaunch; has a Steam page | Reported |
| Persistence | Everything players do leaves a lasting mark: buildings stand until destroyed, dug caves stay, **trees do not respawn unless planted**, "little natural resource regeneration" | Reported |
| Death | Permanent: you lose the character, not experience points | Reported |
| World resets | The map is reset from time to time (reported as about every two years, often out of necessity) | Reported, low confidence |

## Time (the part we care most about)

| Fact | Value | Source quality |
| --- | --- | --- |
| Game year | 180 game days: spring 30, summer 105, autumn 30, winter 15 | Reported (wiki, via search) |
| Real length of a year | 54 days 17 hours 4 minutes (summer 31 d 21 h 57 min; spring and autumn 9 d 2 h 51 min each; winter 4 d 13 h 25 min) | Reported |
| One game day in real time | about **7.3 real hours** | *Derived*: 54 d 17 h 4 min = 1,313 h, divided by 180 |
| Game speed against real time | about **3.3×** real time (24 h per 7.3 h) | *Derived* |
| Crops | Several growth stages, each hours to days; a bee hive makes plants grow faster; the figures are from the old version | Reported, may be outdated |

So the game's own day is about **7 times slower than our usual speed** (1 hour = 1 day) and about 3 times faster than real time. Its long timers are made to be *checked on*, not watched; the world is a shared server that never stops, so there is no catch-up problem to solve there.

![Real time for one game day: ours against Haven and Hearth](img/haven-and-hearth-time.svg)

| Speed | Real time per game day | Real time per game year (365 days) |
| --- | --- | --- |
| Real time (1:1) | 24 h | 365 days |
| Haven and Hearth (reported) | 7.3 h | 54.7 days for its 180-day year |
| Our usual speed (1 h = 1 day) | 1 h | 15.2 days of continuous play |
| 3× | 20 min | 5.1 days |
| 10× | 6 min | 1.5 days |
| 60× | 1 min | 6.1 hours |
| 360× | 10 s | 1 hour |

## Progression and quality (what the game does that we may borrow from)

| System | How it works (reported) | Our counterpart |
| --- | --- | --- |
| **No levels** | Characters grow by attributes (Strength, Agility, Intelligence and others) and by skills, abilities and "credos" | Skills as practice, attributes from the body ([spec 11](../tier0/specs/11-body-composition.md), [spec 13](../tier0/specs/13-capabilities.md)) |
| **Learning Points (LP)** | Earned by discovering and producing new things and by studying "curiosities" over time; spent on skills | Open: knowledge as something a pop holds ([0030](../decisions/0030-pop-continuity-and-capability-loss.md)) |
| **Food Event Points (FEP)** | Eating fills a bar; when full, one attribute rises by one, chosen at random in proportion to the food's FEP split (example: a food that is 4 parts Strength to 3 parts Charisma gives 57.1 % and 42.9 %). The bar needs as many points as your highest attribute; **eating varied foods lowers the cost** | H&H abstracts nutrition into points. Ours should come from the body (nutrients, kg), so variety matters for a real reason, not a bar. Idea only |
| **Quality (Q) with softcaps** | A product's maximum quality comes from an attribute and a skill, for example `sqrt(Dexterity × Sewing)` for leather and silk, `sqrt(Strength × Smithing)` for smithing; Marksmanship softcaps bows; Masonry affects digging stone and clay | Our quality system ties everything built to one quality number ([items and crafting](../design/items-crafting.md), [0013](../decisions/0013-production-ladder.md)). A geometric mean of the person's body and skill is a candidate form to compare in a later pass |
| **Convenience** | A skill that speeds civil actions such as crafting and digging | A modifier on a Process's rate (focus and compression §9) |
| **Terraforming** | Dig, pave, plow, plant; caves over several levels | Zones and deposits ([world systems](../design/world-systems.md)) |

## The loop that matters to us

The core loop reported by reviews: start a **timed action** (dig, craft, harvest, cook), watch a **progress bar** fill, do something else or wait; many long processes (curing, growing, smelting) run in the background while you do other things. This is the "loop animation with a progress bar" of [0042](../decisions/0042-real-durations-played-as-progress-bars-with-a-speed-ladder.md). The reported strengths are persistence, the real cost of everything, and a world that remembers; the reported cost is long waits and grind, which is why our speed ladder and mini-games exist.

## What we take, and what we do not (Proposal)

| Take | Do not take |
| --- | --- |
| Real durations, with timed actions as the unit of play (a Process) | A shared always-on server: ours is single-player, time stops when closed (0043) |
| Many processes running at once, queued by the player | Abstract nutrition points |
| A world that remembers and does not regenerate for free (matches "nothing is created or destroyed") | Random world resets as a design feature |
| Quality from the person times the craft, with softcaps | The client code: it is LGPL, our engine is our own ([0002](../decisions/0002-engine-stack.md)); read it for ideas only |
| Permadeath as an option (our survivors already die for real) | |

## Not found (to check later)

- Official or wiki pages on actions, timers and the quality formula (blocked), so no exact crop, curing or smelting times and no full quality formula.
- What happens to a character that logs out (I recall characters staying in the world for a short time, unverified).
- The current (2024 and later) state of the game, apart from its Steam page.
- The licence text of the client repository.

## Sources

- Ring of Brodgar wiki (the community wiki; **blocked, search summaries only**): [About](https://ringofbrodgar.com/wiki/About_Haven_and_Hearth), [Skills (legacy)](https://ringofbrodgar.com/wiki/Legacy:Skills), [FEP](https://ringofbrodgar.com/wiki/FEP), [Crops](https://ringofbrodgar.com/wiki/Crops), [Crops table](https://ringofbrodgar.com/wiki/Crops_table), [Autumn](https://ringofbrodgar.com/wiki/Autumn) (the day and year figures).
- Client source mirror: [hafen-client on GitHub](https://github.com/Fr-Dae/hafen-client).
- News: [TechRaptor on the 2015 overhaul](https://techraptor.net/gaming/news/haven-hearth-receive-major-overhaul), [Massively OP on the relaunch](https://massivelyop.com/?p=22764); [TV Tropes entry](https://www.tvtropes.org/pmwiki/pmwiki.php/VideoGame/HavenAndHearth); [Steam page discussions](https://steamcommunity.com/app/3051280/discussions/0/510696076432383198).
