# The story explorer: navigating the world to find its stories

**Date:** 2026-10-09. **Status:** Proposal (design), from the user's request of 2026-10-09. Its ground rules (§2) are decision [0067](../decisions/0067-stories-are-found-never-written.md). **Builds on:** [0061](../decisions/0061-the-world-runs-without-the-player.md), [0062](../decisions/0062-the-game-loop-attention-possession-and-late-starts.md), [0048](../decisions/0048-realistic-time-default-commit-on-observation-and-pins.md) (pins), [macro layer](macro-layer.md) (headlines and Chronicle, "detected, never injected"), [spec 25](../tier0/specs/25-chronicle.md) (chronicle, built), [spec 39](../tier0/specs/39-orders-and-belief.md) (beliefs and Tell, built), [spec 28](../tier0/specs/28-pop-levels.md) (lazy pops, ranks, invertible ids), [content tools](content-tools.md) (lifepaths with places), the research notes [LB](../research/library-of-babel-and-sacred-geometry.md) and [PKC](../research/playing-knowing-commanding-moving.md), and the detachment levels of [0066](../decisions/0066-player-states-embodied-detached-observer.md).

## The user's words (verbatim, lightly cleaned)

> "I want the user to be able to navigate through the world it simulated to see and find things, like the people who use Google Earth that way. Random situations born of the sim, awe behaviour or intriguing stories one could tie by knowing who knew who and did what. Like watching or reading a show, where you see the different characters of different small or big stories, all tied with coherence with the world and places."

## 1. In one paragraph

The **explorer** is the observer's main tool. It looks like Google Earth, but its pins are **stories the simulation actually produced**. A **librarian** (a story sifter, [LB-16](../research/library-of-babel-and-sacred-geometry.md), [LB-18](../research/library-of-babel-and-sacred-geometry.md)) finds them in the event log as they form. Each story is tied to real people, real places and real causes. You can:
- **read** it like a book;
- **watch** it like an ensemble show, cutting between its characters live;
- **follow the strings** between people on a board (who knew whom, and how);
- **jump in** and play as anyone in it, when your permission allows.

The shape is **hyperlink cinema** ([LB-50](../research/library-of-babel-and-sacred-geometry.md)): separate stories whose links are slowly revealed, like the four stories of the film *Babel* tied by one rifle. Our links are always real: a shared person, place, object or cause.

## 2. Ground rules (decision [0067](../decisions/0067-stories-are-found-never-written.md))

1. **Found, never written.** Every story is made of facts the simulation already holds: events from the log, derived facts (pure functions of seed and id), and beliefs. The explorer adds no event, person or motive (0061; macro layer "detected, never injected").
2. **Every fact has a place and a time.** Each fact records who, what, where, when, cause and source:
   - **where** is a named place: a building, street, business, town or cell from the generators;
   - **when** is a tick.

   Stories are drawn on the map, and two consecutive facts must be reachable in the time between them.
3. **Exploring changes nothing.** Reading, clicking, searching and the board never change state (0042, 0061). Attention raises detail for the present and future only (0062 rule 4).
4. **The past is shown at the detail it was lived:**
   - **A place that ran cold has a list, not a scene.** It shows counts plus the named people derived behind them.
   - **"Watch it happen" exists only where detail ran.**
   - **Dramatising is optional and labelled.** It may fill in *how* (positions, gestures) inside recorded facts, never *who* or *what*. It is off by default.
5. **No future.** Stories come only from the committed past ("never show the future" in the run-ahead draft, RA section 5).
6. **What you may see follows your detachment level** ([0066](../decisions/0066-player-states-embodied-detached-observer.md); [PKC](../research/playing-knowing-commanding-moving.md) player states):
   - **free:** the truth;
   - **knowledge:** only stories your pop or side knows, built from beliefs, which can be wrong;
   - **off:** only your own body's diary.

## 3. Where stories come from

An 8-billion-pop world cannot run everyone in detail, so stories have three sources. Each is honest about its resolution.

| Source | Where | What a story looks like there | Built from |
| --- | --- | --- | --- |
| **Lived in detail** | Places that ran hot or warm: the player's attention, records' life jumps, and the **world's own story spots** (below) | Full scenes: who did what to whom, with cause chains; can be watched | Event log with causes (built), chronicle (spec 25, built) |
| **The coarse past, made personal** | Everywhere else | "Lyon, 3rd district, day 12: 340 dead". Opening it lists the 340 by name: the members whose rank fell below the count (spec 28 rule 2). Their families and coworkers are derived on demand (§4) | Macro events, `CohortPermutation`, invertible ids ([LB-21](../research/library-of-babel-and-sacred-geometry.md), [LB-22](../research/library-of-babel-and-sacred-geometry.md)) |
| **Told** | Wherever people talk | Rumours, news and orders: who told whom, after how many retellings, and how far it drifted from the truth | Beliefs and `Tell` (spec 39, built) |

**The world's own story spots (new).**
- **What they are:** a fixed share of the detail budget the simulation spends by its own rule, chosen from world state and never from the player. So rich stories happen even where nobody looks. Because the choice is deterministic, a run with no player has the same spots (0061's standing test), and every viewer and replay sees the same stories.
- **How a spot is chosen**, by cheap macro signals:
  - **contested places:** the outbreak front, sieges, horde paths;
  - **records in danger:** officeholders and leaders;
  - **surprises:** a cell doing far better or worse than its model predicted;
  - **crossings:** places where flows of people meet;
  - **ordinary life:** a keyed lottery among ordinary places, so ordinary life gets stories too.
- **Spots stick for days**, with hysteresis, because a story needs continuity.
- **Default budget:** about a quarter of the embodied budget, for example 2 hot spots and 16 warm ones. This is an estimate, to be measured with the spec 28 benchmark.
- **Precedent:** The Sims 4's Neighborhood Stories changes households nobody plays (PKC-15). Ours keeps the rules identical to played places.

## 4. Who knew whom: ties

Every string on the board has a reason you can click. There are four kinds of tie:

| Tie | Examples | Source | Stored? |
| --- | --- | --- | --- |
| **Family** | parent, child, sibling, spouse | Invertible ids and households (spec 28) | No: derived |
| **Shared place** | household, coworkers, classmates, neighbours | Lifepath terms: same place in overlapping years. This is the "ties to other pops met in a shared workplace or school" that [content tools](content-tools.md) §4.1 lists for later. Coworkers come from the workplace roster; classmates from school plus birth year | No: derived |
| **Did something together** | told, traded, fought, saved, bit, ordered, hid something from | Events in the log | Yes, on records, with the event id |
| **Knows about** | A believes something about B | Belief store lens (spec 39) | Yes (already) |

- **A label for every string:** for example "worked together at Pete's Diner, 2019–2024" or "told her about the cordon on day 3 (2 hops, drifted 30 %)".
- **"How are these two connected?"** is a shortest path over ties, limited to a few hops. It is the six-degrees game played on a real world: the president and a survivor in a basement. Derived ties are bounded per person, so a two-sided search a few hops deep is affordable. The depth limit is an estimate to measure.
- **Search runs backwards** ([LB-36](../research/library-of-babel-and-sacred-geometry.md)):
  - "every nurse in Lyon" or "who knew Marta" enumerate rosters, ranks and derived ties;
  - no lazy pop is generated except the ones on the answer list.
- **Ours beats Watch Dogs: Legion's Census** ([LB-48](../research/library-of-babel-and-sacred-geometry.md)). Census invents a person's identity when the player profiles them, then saves it. Ours exists before anyone looks, and it agrees with the world's totals.

## 5. Stories (threads) and the librarian

**A thread** has:
- a pattern kind;
- a title from a template filled with facts;
- a cast with roles;
- its places and its events, in order;
- a status: forming, ongoing or ended;
- a score;
- its crossings with other threads.

**The librarian** runs patterns incrementally over the event stream ([LB-18](../research/library-of-babel-and-sacred-geometry.md)):
- each pattern is a small state machine that joins events on shared people, places and causes;
- a partial match is a **forming** thread, shown pulsing on the map;
- matches that can no longer complete are dropped, so memory stays bounded;
- patterns are data files, so modders can add their own.

**Default patterns**, all grounded in systems we have or have specified:

| # | Pattern | What it matches | Needs | Status of what it needs |
| --- | --- | --- | --- | --- |
| 1 | **A death with a cause** | A death and its chain back to the root ("a cut on day 6, never cleaned") | Spec 25 | Built |
| 2 | **The hidden bite** | Someone hid a bite; who paid for it | Spec 25 danger tag | Built |
| 3 | **The rumour** | A belief that travelled at least 3 hops, drifted far from the truth, and that someone acted on | Spec 39 | Built |
| 4 | **The order** | An order traced from a soldier up to a minister and its cause (demo flow, second flow) | Military orders, chain of command | Decided in outline |
| 5 | **The butterfly** | A small root event with a large set of descendants: the generator left running that brought the horde | Cause chains | Built (a count over the log) |
| 6 | **Against the odds** | An outcome the simulation itself rated unlikely (§6) | Probability carried on events | New |
| 7 | **The holdout** | A place with people under threat for N days that ends in a fall or a relief | Sieges, hordes | Specified in part |
| 8 | **Apart and searching** | Family split since the outbreak; one moves toward the other; they meet or they don't | Derived ties, movement | New |
| 9 | **Crossed paths** | Strangers whose paths cross twice in different places, or old classmates on opposite sides | Derived ties, events | New |
| 10 | **Rise and fall** | A pop gains office or leads a group; a leader loses it | Offices, leaders (PKC) | Decided in outline |
| 11 | **The exodus** | A group moving across cells together | Flows (HST-143) | Researched |
| 12 | **The case** | A secret act whose traces survive with enough solvability ([cases-and-living-lore](cases-and-living-lore.md) §8, §9) | Secret acts, traces | New |

These are the first shapes. The full set for every kind of story (love, loss, adventure, thrill, action, redemption, small joys) is in [story-blocks](story-blocks.md) §4. The blocks there write intentions and words, never outcomes.

**Spectacles** are world-scale sights with no single cast:
- a city going dark as its grid fails;
- a horde wave turning;
- a river of refugees;
- a fire front;
- the last lights of a country.

They are shown as **tours**, like Google Earth's Voyager ([LB-45](../research/library-of-babel-and-sacred-geometry.md)): a sequence of about ten places with a caption at each.

**Crossings** are the hyperlinks: two threads that share a person, place, object or cause. A story card says "this story touches 2 others", and following a crossing is how a viewer ties the show together.

## 6. Scoring: what is worth showing

- **Stakes:** people affected and deaths.
- **Surprise, from the simulation's own odds.** Every keyed chance has a known probability p, so a system that resolves one can carry p on the event. The surprise is −log₂ p bits:
  - a coin-flip death is worth 1 bit;
  - surviving 2 % odds is worth 5.6 bits.

  A thread's surprise is the sum over its key outcomes. "Intriguing" then has a number, and a viewer can be told why a story was picked.
- **Ties:** casts that know each other score higher than strangers.
- **Place:** named, known places and records present.
- **Recency.**
- **Variety:** a quota per pattern kind, so the list is never all deaths. This is the oatmeal rule ([LB-41](../research/library-of-babel-and-sacred-geometry.md)).

## 7. What the viewer sees

| Part | What it does | Precedent |
| --- | --- | --- |
| **Map** | Globe to street with layers; story pins (icon by kind, size by score, pulsing while forming); spectacle tours; a time slider for the past at each layer's stored detail; back and forward history like a browser (Google Earth has none, PKC-35) | Google Earth ([LB-45](../research/library-of-babel-and-sacred-geometry.md)) |
| **Lucky** (a die button) | Jumps to a good thread, weighted by score and variety. Google's draws from 20,000 hand-picked places; ours draws from the librarian's finds | [LB-45](../research/library-of-babel-and-sacred-geometry.md) |
| **Search** | People by name, job or place; "who knew X"; "stories in Lyon this week" | §4 |
| **Story card** | Title, a short summary in chronicle sentences, cast portraits, places on the map, timeline, crossings. Buttons: Read, Watch, Follow, Board, Share | Google Earth knowledge cards ([LB-45](../research/library-of-babel-and-sacred-geometry.md)) |
| **Reader** | The thread as chapters by day, stitched from the cast's diaries one point of view after another ([story-blocks](story-blocks.md) §6); every name and place clickable; a "previously on…" recap when a followed thread moves on | Spec 25 sentences; Dwarf Fortress Legends ([LB-46](../research/library-of-babel-and-sacred-geometry.md)) |
| **Watch** (director camera) | Live intercutting between the current moments of the threads you follow: it cuts on events and captions names and ties, like an ensemble show. Only where detail runs; never the future | Hyperlink cinema ([LB-50](../research/library-of-babel-and-sacred-geometry.md)) |
| **Board** | Who knew whom, as a corkboard. Red strings are ties, each with its reason. Expand one hop at a time; filter by tie kind; "how are these two connected?" In knowledge mode it holds only what your pop knows, and you pin and string things yourself. Like Shadows of Doubt, the board never makes the leaps for you | Shadows of Doubt ([LB-47](../research/library-of-babel-and-sacred-geometry.md)) |
| **Person card** | The inspector's tabs (Life, Body, Mind, Ties, Things, Know-how; [world simulation](world-simulation.md)), a **Diary** tab (the pop's story up to now, one page per day: [story-blocks](story-blocks.md) §6) and "Appears in" stories; Locate, Follow, Play as | Unlike Cities: Skylines II, whose journal starts only when you follow (PKC-29): ours has the whole past, derived |
| **Share** | A call number: build and data hashes, seed, tick and thread id. Anyone on the same build opens the same story ([LB-38](../research/library-of-babel-and-sacred-geometry.md)) | Google Earth's share; Babel's bookmarks |

**In knowledge mode the explorer becomes the survivor's notebook.** Stories come from what the pop was told and saw, with sources and ages (spec 39 lens; PKC belief symbols). Finding someone means asking people who knew them. That is exactly *Bad News* ([LB-49](../research/library-of-babel-and-sacred-geometry.md)), where a player finds a dead man's next of kin in a simulated town. It is also "find my family" ([LB-43](../research/library-of-babel-and-sacred-geometry.md)) as gameplay.

## 8. Tests (coherence)

- Every event a thread cites exists in the log, and every fact has a place and a tick. (A1)
- Consecutive facts of one person are reachable in the time between them. (A1)
- A run with no player and a watched run choose the same story spots. Threads agree where detail was the same. (A1, 0061)
- No thread cites a tick later than the committed tick. (A1)
- Derived ties are symmetric and stable across calls and across save and load. (A1)
- The people named behind a cold count are exactly as many as the count. (A1)
- Over N runs, no pattern kind holds more than its quota of the top list. (A2)

## 9. Build order (simulation first, the window only to show it)

1. **Thread records and patterns 1, 2 and 5** on the built chronicle and cause chains, headless: `zs stories` prints a run's threads with their scores and crossings, inside the one executable.
2. **Derived ties** from households, ids and lifepaths, plus `zs who-knew <pop>`.
3. **Probability on chance events**, and the surprise score (pattern 6).
4. **Rumour and order patterns** (3, 4) on spec 39.
5. **Story spots**, with the chunk work (spec 27).
6. **The 2D window:** a Stories panel (list, Reader, clickable names, Locate). This is the one window change, because it shows a simulation feature.
7. **With the renderer:** map pins, Lucky, Board, Watch and tours.

Steps 1–4 need no generator or planet work.

## 10. Defaults taken (no open items)

- **Story spots:** on by default, at about a quarter of the embodied budget until the benchmark says otherwise.
- **Patterns:** data files; the 11 above ship built in.
- **Text:** template sentences in the spec 25 style. No generated prose adds facts.
- **Dramatising the cold past:** off by default; labelled when on.
- **Explorer:**
  - **in free mode:** the truth and every tool;
  - **in knowledge mode:** the notebook;
  - **in off mode:** the body's diary only.
- **Pins** (0048) work on threads as well as on pops and places. A pinned thread shows "previously on…" when it moves on.
- **Clickable names** are [content tools](content-tools.md) §4.2, "mentions are links", used everywhere in the explorer.
  - **Opening a person derives them and promotes no one,** as [pop continuity](pop-continuity.md) says. §4.2 once said a click "promotes it to a record if needed"; it was corrected on 2026-10-10 ([0067](../decisions/0067-stories-are-found-never-written.md)).

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [procedural-mysteries](../research/procedural-mysteries.md): Procedural mysteries: cases that come from a simulation
- [story-building-blocks](../research/story-building-blocks.md): Story building blocks: how games and story theory build variety from parts
- [lore-made-and-generated-worlds](../research/lore-made-and-generated-worlds.md): Lore in made and generated worlds: Project Zomboid, other games, and tools outside games
