# Story blocks: pre-written parts for every kind of story, and every pop's diary

**Date:** 2026-10-09. **Status:** Proposal (design), from the user's requests of 2026-10-09 and 2026-10-10. Its rule (§2) is part of decision [0067](../decisions/0067-stories-are-found-never-written.md). **Builds on:** [story-explorer](story-explorer.md) (the rules "found, never written" and "the past at the detail it was lived"), [cases-and-living-lore](cases-and-living-lore.md) (hooks, secret acts, traces), [inhabitants](inhabitants.md) (bonds, background), [content tools](content-tools.md) (culture packs, lifepath), [spec 24](../tier0/specs/24-pop-ai.md), [spec 25](../tier0/specs/25-chronicle.md) (day summary), [spec 28](../tier0/specs/28-pop-levels.md), [spec 39](../tier0/specs/39-orders-and-belief.md), and the research in [story-building-blocks](../research/story-building-blocks.md) (SB).

## The user's words (verbatim, lightly cleaned)

> "Lore-wise, not only detective, crime and conspiracy stories: it's for everything: love, thrill, sad, action, adventure, etc. Maybe we can have building blocks for different types of things or contexts, like pre-written things that, put together in different forms, could make a certain variety of stories?"
>
> "Idea: every pop has a diary where the story of that pop up to the moment is. Maybe a daily report?"

## 1. In one paragraph

Yes: pre-written blocks, combined by the simulation. The blocks go into:
- what people **carry** from the past (seeds);
- what they **want** and how they try (wants);
- what a **place** makes possible (places);
- how we **recognise** a story once it has happened (shapes);
- how we **tell** it (lines).

**The blocks never decide what happens.** The world decides whether she makes it to Porto, whether he is still waiting, whether the bite was hidden. That keeps every story true to the simulation, and it gives far more variety than the blocks alone. An outcome nobody wrote cannot repeat the way a written event does ([SB-17](../research/story-building-blocks.md)).

**Every pop gets a diary**, rendered on demand from the facts and beliefs it already has, so it costs no storage.

## 2. The rule: blocks write intentions and words, never outcomes

| | A storylet elsewhere (Fallen London, Wildermyth) | A block here |
| --- | --- | --- |
| Has prerequisites | Yes ([SB-4](../research/story-building-blocks.md)) | Yes: casting criteria on real pops, places and world state |
| Has content | Yes | Yes: a want, a recogniser, or lines of text |
| Has **effects** on the world | Yes: relationship points, items, outcomes ([SB-7](../research/story-building-blocks.md)) | **No.** It changes the world only through actions pops choose under the normal rules ([SB-13](../research/story-building-blocks.md)) |

- **An example:** a Wildermyth-style event can say "the two heroes fall in love: +20 romance".
- **Ours can only say:** "these two may want to court each other". After that, proximity, shared danger, help given and each one's traits move their affinity by the bond rules ([SB-11](../research/story-building-blocks.md)). Healing raises trust or debt, not affection.
- **Love can grow, stall or turn into resentment.** The story is whatever happened.

## 3. The five kinds of block, plus one computed

| Kind | What an author writes | Example | Decided by |
| --- | --- | --- | --- |
| **Seed** (the past) | A hook kind with roles, casting criteria, residue and the wants it can wake. Lifepaths cast it on real pops. Cases' hooks are the crime-flavoured seeds ([cases-and-living-lore](cases-and-living-lore.md) §4) | *Old flame:* A and B were engaged; B moved away for work in the last 5 years. Residue: letters in A's drawer, a ring. Can wake: Reunite, Court | The seed (pure function of the world seed) |
| **Want** (intention) | A goal with roles, waking conditions, a plan of steps, what each step needs, and how it can fail | *Reunite:* seeker and sought, separated, the seeker believes the sought is alive. Steps: learn where → get transport or walk → cross what is between → arrive. Fails on: death, giving up as hope drops, a new bond | Pop AI picks it by utility; the world resolves each step |
| **Place** (context) | What a kind of place affords: roles, activities, the wants it can wake, the moods it colours | *Hospital:* triage, nurse and patient bonds, medicine to steal, deaths to witness. *Checkpoint:* separations, bribes, papers. *Church:* weddings, funerals, sanctuary | The generators place them; the simulation uses them |
| **Shape** (recogniser) | A beat pattern the librarian matches over events, with roles. Beats are Propp-like functions: an act defined by what it does for the story, whoever does it ([SB-1](../research/story-building-blocks.md)) | *Bond under fire:* two pops in the same danger, one helps the other, affinity crosses a threshold. *Too late:* a seeker arrives after the sought died or left. *The sacrifice:* a pop's act saves others at a cost to itself | The librarian ([story-explorer](story-explorer.md) §5) |
| **Line** (telling) | Text for a situation, with criteria. The most specific match wins and general lines are fallbacks (Valve, [SB-6](../research/story-building-blocks.md)); grammar variants à la Tracery ([SB-9](../research/story-building-blocks.md)). Lines may say only what their criteria prove | General: "{A} reached {place}." More specific, when A walked more than 100 km: "{A} walked {km} km to reach {place}." Most specific, when B is now with C: "{A} walked {km} km to reach {B}. {B} opened the door holding {C}'s hand." | Matching on facts |
| **Arc** (computed, not written) | Nobody writes it: a pop's fortune over time (safety, health, bonds, hope) is matched to the six shapes of Reagan et al. ([SB-3](../research/story-building-blocks.md)) | Man in a hole (lost everything, rebuilt); Icarus (rose to lead the shelter, fell); Cinderella | Measured from the simulation ([SB-15](../research/story-building-blocks.md)) |

**Three more kinds, taken from Project Zomboid and RimWorld** ([lore-made-and-generated-worlds](../research/lore-made-and-generated-worlds.md)):

| Kind | What an author writes | Chosen by | Source |
| --- | --- | --- | --- |
| **Scene** | How a place looks after a known kind of event: bodies, barricades, a note, what was taken | The facts the simulation holds about that place. A household lost to a hidden bite gets the barricaded-bedroom scene. PZ's randomized scenes, but never by dice | [LR-4](../research/lore-made-and-generated-worlds.md), [LR-20](../research/lore-made-and-generated-worlds.md) |
| **Broadcast** | TV and radio lines for real events and for each office's official version, which can lie | Events and edicts (spec 39). Hidden stations are real pops with transmitters, and the last government loop plays when the power goes | [LR-2](../research/lore-made-and-generated-worlds.md), [LR-21](../research/lore-made-and-generated-worlds.md) |
| **Object that remembers** | Graves, memorials, graffiti, messages for missing family, photos | A pop's action, pointing at real events and people (RimWorld's art tales) | [LR-13](../research/lore-made-and-generated-worlds.md), [LR-22](../research/lore-made-and-generated-worlds.md) |

**Above all the blocks sits the premise:** what the outbreak is and its rules. It is written by hand, as PZ writes its Knox Event, and kept as sandbox parameters (PZ calls them "Zombie Lore"). Optional authored origin presets give a canonical lore ([LR-23](../research/lore-made-and-generated-worlds.md)).

**Casting is Wildermyth's** ([SB-7](../research/story-building-blocks.md), [SB-14](../research/story-building-blocks.md)). A seed, want or shape names its roles and their criteria, and fills them only from real pops. If nobody fits, it does not happen.

## 4. Kinds of story, and the blocks that make them

| Kind | Typical seeds | Typical wants | Typical places | Shapes the librarian finds | Usual arcs |
| --- | --- | --- | --- | --- | --- |
| **Love** | Old flame, unrequited, married with distance between them | Court, Confess, Protect, Reunite, Choose (love or duty) | Shelter, hospital, convoy | Bond under fire; triangle; the choice; reunion | Cinderella, man in a hole |
| **Sadness and loss** | A child abroad, an unfinished goodbye, a sick parent | Bury properly, Keep a promise to the dead, Find what happened | Cemetery, home, hospital | Too late; putting down a loved one (Polti no. 23, [SB-16](../research/story-building-blocks.md)); the empty house | Tragedy, Oedipus |
| **Adventure** | A map, a boat, a cabin in the mountains, a sibling abroad | Reach (a place or a person), Escape the city, Explore | Roads, borders, ports, wilderness | The journey; crossing; the return | Man in a hole, rags to riches |
| **Thrill** | A debt to dangerous people, a secret | Hide, Run, Steal, Outwit | Black market, checkpoint, ruins at night | The chase; the narrow escape; the double-cross | Icarus, Oedipus |
| **Action** | Military service, a rival, a weapon skill | Hold this place, Rescue, Strike, Lead | Barricade, bridge, depot | The last stand; the rescue; the breakout | Any |
| **Mystery and crime** | Debts, grudges, secrets, old crimes | Steal, Hide an act, Scheme, Investigate | See [cases-and-living-lore](cases-and-living-lore.md) | The case | Any |
| **Redemption** | A past failure (deserted, caused a death) | Make amends, Protect the ones failed before | Wherever the failure happened | The second chance | Man in a hole, Cinderella |
| **Small joys** | A pet, a hobby, a birthday | Celebrate, Teach, Share food | Shelter, kitchen, garden | A birthday in the shelter; the first harvest; the dog adopted | Rags to riches (small) |

A thread can match several shapes, so a story can be love and adventure and loss at once.

## 5. One story from blocks (nothing scripted)

1. **The seed (lifepath):** Marta and Tomás were engaged in 2023. He moved to Porto for work. Her drawer holds his letters.
2. **The pressure (simulation):** the outbreak, a cordon, phones down.
3. **The want:** Reunite wakes in Marta, because she believes Tomás is alive (a rumour by radio, spec 39).
4. **The steps, resolved by the world:**
   - she trades her grandfather's watch for fuel at a black market (a thrill beat);
   - she crosses the cordon at night and is hurt (action);
   - the car dies, and she walks.
5. **Meanwhile, in Porto:** Tomás, wounded, is treated by a nurse. His trust grows, then affection (bond rules, not a script).
6. **Day 41:** Marta arrives.
7. **What the librarian finds:** the journey, bond under fire (Tomás and the nurse), too late. The arc for Marta is Icarus.
8. **The line it picks:** "Marta walked 312 km to reach Tomás. He opened the door holding Inês's hand."

The next run of the same seed, with one choice changed, can end in a wedding in a shelter.

## 6. Every pop's diary (the user's idea)

**Every pop has a diary: the story of that pop up to now, one page per day.** It is not stored text. It is rendered when someone opens it, from what the simulation already knows, with lines (§3). So 8 billion diaries cost nothing until read, and each page reads the same every time: the line choice is keyed by pop and day.

### What a page holds

- **Where** the pop was (schedule, moves).
- **Who** they met (ties).
- **What** they did and what happened to them (events with causes).
- **What they learned** (beliefs added that day, with source).
- **How they were:** the change in safety, health, bonds and hope (the fortune curve that feeds arcs).
- **The day's moment:** the highest-scoring event, as spec 25's day summary already scores the group's day (death 10, someone joined 6, broadcast heard 8…). Spec 25 does it for the group; this does it for each pop.

### Dull days fold

Like a real diary, quiet stretches compress: "Weeks 3–5: worked at the bakery, slept at home." A day with a high-scoring moment gets its own page. This keeps the oatmeal away ([LB-41](../research/library-of-babel-and-sacred-geometry.md)) and makes the important days stand out.

### Pages show the detail they were lived at

This is the explorer's rule 4 ([story-explorer](story-explorer.md) §2):

| The pop was | Its pages say |
| --- | --- |
| Embodied (hot) | Hour by hour, with the scenes |
| A record (warm) | Day by day, from its life jumps and events |
| Lazy (cold) | Only the big things, derived when opened (spec 28 random-access life): born, moved, married, job lost, infected, died, plus the macro days that touched them ("the day the district fell"). Everything else folds into ordinary weeks |

### Two voices for the same day

- **The report** (the truth): what happened. Shown in free mode, and to anyone for their own pop.
- **The diary** (the pop's own voice): what the pop believed and felt, in lines fitted to their traits. It can be wrong (they were lied to), selective (a secretive pop leaves things out) and moody.
  - Pops with a writing habit (a trait) keep a **physical diary**, an item in the world. It is a trace for cases ([cases-and-living-lore](cases-and-living-lore.md) §6) and a treasure for whoever finds it.
  - Only physical diaries store anything: per day, references to the facts and the claim as believed then. That is tens of bytes a day, for record pops with the habit only, because beliefs change later and the diary must keep what they believed on that day.

### What can be read in each mode

- **Free mode:** anyone's report and diary.
- **Knowledge mode:**
  - your own pop's diary;
  - physical diaries you have found;
  - what people tell you about themselves.
- **Off mode:** your own diary only.

### The same idea one level up

Every entity with a story has a daily page:
- a **place**: the town's day summary;
- a **group**: the shelter's log;
- an **office**: the minister's daily briefing;
- a **country**: its Chronicle and headlines (macro layer).

### Reading many diaries is reading the show

The explorer's **Reader** can stitch the cast's diaries into chapters, one point of view after another: "Day 12, Marta: … / Day 12, Tomás: …". It is the epistolary form of *Dracula*, and the survivor interviews of *World War Z*, made from a live world.

## 7. Writing blocks

- **Data files, like culture packs** ([SB-12](../research/story-building-blocks.md)): one file per block family, and moddable. A culture pack can carry its own variants: courtship customs, funeral rites, how people write a diary.
- **Additive authoring:**
  - a writer adds a more specific line without touching the general ones (Valve's cascade, [SB-6](../research/story-building-blocks.md));
  - a want adds a new failure branch without breaking old ones;
  - a shape adds a beat.
- **Each block names the systems it needs.** A block whose systems are missing is inactive, not broken. For example, "Court" needs bonds, and "the chase" needs vehicles.
- **A starter set:** about 30 seeds, 20 wants, 15 places, 25 shapes and a few hundred lines. That is an estimate of what one writer can do in a few weeks. Variety comes from the world's outcomes, not from piling up blocks ([SB-17](../research/story-building-blocks.md)).
- **Repetition is measured, not guessed:**
  - how often each line is used over N runs;
  - how often two threads get the same title;
  - the oatmeal test for diaries (§6).

  Wildermyth's critics are the warning ([SB-8](../research/story-building-blocks.md)).

## 8. Build order (simulation first)

1. **The diary, first slice, headless:** a per-pop day page from the Tier 0 run. It extends spec 25's day summary to each pop, uses general lines, and is printed by `zs diary <pop>`. Everything it reads is built.
2. **Lines with criteria and the most-specific rule,** used by the chronicle sentences, the diary and story titles.
3. **Seeds:** the lifepath hooks of the cases draft, with the genre seeds of §4. A data and generator change.
4. **Wants:** plan templates for pop AI (spec 24 and the pop behaviour model). First Reunite and Protect, because Tier 0 already has a group and the stranger.
5. **The fortune curve and arcs,** from pop state.
6. **Shapes in the librarian** (story explorer §5), with genre tags.
7. **Places' affordances,** with the city and building generators.
8. **Physical diaries** as items and traces (cases draft §6).

## 9. Defaults taken (no open items)

- **Blocks never carry effects;** only actions do (§2).
- **Diaries:**
  - every pop has one, rendered on demand and stored only as physical diaries;
  - quiet days fold;
  - the line choice is keyed by (pop, day).
- **Two voices:** the report (truth) and the diary (belief). Your own diary is always readable.
- **Blocks are data files,** with culture variants.
- **Tone is computed from the arc** and never written into a block.
