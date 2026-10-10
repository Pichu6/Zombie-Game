# Cases and living lore: mysteries that come from the simulation

**Date:** 2026-10-09. **Status:** Proposal (design), from the user's request of 2026-10-09. Its ground rule (§3) is part of decision [0067](../decisions/0067-stories-are-found-never-written.md). **Builds on:** [inhabitants](inhabitants.md) (fact, memory, evidence and account; background residue; outbreak origin), [content tools](content-tools.md) §4.1 and §4.2, [spec 24](../tier0/specs/24-pop-ai.md), [spec 25](../tier0/specs/25-chronicle.md), [spec 32](../tier0/specs/32-stocks-goods-and-money.md), [spec 39](../tier0/specs/39-orders-and-belief.md), [0048](../decisions/0048-realistic-time-default-commit-on-observation-and-pins.md) (commit on observation), [0058](../decisions/0058-offices-open-layers-single-player-is-multiplayer-with-ai.md) (stations), [story-explorer](story-explorer.md), and the research in [procedural-mysteries](../research/procedural-mysteries.md) (MY).

## The user's words (verbatim, lightly cleaned)

> "I would like the user to be able to find mysteries or solve cases from different sources of information: a note on character A and another on character B. Perhaps seeing a crime being committed and following the criminal, and that ends up leading to a conspiracy or a crime detective story."
>
> "I take the lore as background mostly, but don't know how it could be procedural during the sim, so that when zoomed in enough there's logic between everything. That discovery feeling."

## 1. In one paragraph

A **case** is a real chain of events whose key facts the people who did them want hidden:
- the **simulation** makes the chain: pops with motives choose secret acts;
- the **world** keeps what the acts left: traces in places, documents, ledgers and memories;
- **people** hold partial memories, some of them false, and may lie;
- the **player** rebuilds the chain from the pieces: a note on A, a diary entry on B, a neighbour who heard a car at 2 a.m., a stock count that does not add up.

Nothing is authored. Every clue is a projection of something that really happened, so when you zoom in everything has a reason ([MY-19](../research/procedural-mysteries.md)). **Lore stops being backdrop:** a pop's past leaves **hooks** (a debt, a grudge, a secret, a favour owed), and pressure from the simulation can wake them. Most never wake, and that is correct.

## 2. What we already have

| Piece | Where | What it gives a case |
| --- | --- | --- |
| Fact, memory, evidence and account as four separate layers | [inhabitants](inhabitants.md) ([MY-13](../research/procedural-mysteries.md)) | The exact split a mystery needs: truth, what each person perceived, what objects show, and what people say |
| "Every background step leaves verifiable residue" | inhabitants, background | Lore that can be checked in the world |
| Outbreak origin: fixed truth, then witnesses, cover-up and public version | inhabitants, outbreak origin | The biggest case in the game, already intended |
| A secretive pop hides a bite and says "just a cut" | spec 24 rule 7 ([MY-14](../research/procedural-mysteries.md)) | Our first secret act and our first lie |
| Corpses rot in phases; flies reveal hidden bodies from over 100 m | [body trauma](body-trauma.md) ([MY-15](../research/procedural-mysteries.md)) | Physical traces with a clock |
| An exact ledger with no hidden mint | spec 32 ([MY-16](../research/procedural-mysteries.md)) | A theft always leaves a gap somewhere |
| Beliefs, retelling and distortion; who told whom | spec 39 | Rumours and statements with sources |
| Noise and light fields | specs 18 and 19 | Who could have heard or seen it |
| Clickable names that point at real census slots | content tools §4.2 | Every name in a note leads to a real person |
| Lifepath terms with places | content tools §4.1 | Who worked, studied or lived where, and when |

**What is missing:**
- secret acts beyond the bite;
- traces as things in the world;
- hooks with a second person and residue;
- statements that can lie;
- the case board.

## 3. The chain: truth → acts → traces → knowledge → deduction

1. **Truth.** Hidden facts: motives, secret acts, secret ties. They come from hooks (§4) and the simulation.
2. **Acts.** Secret actions chosen by pop AI (§5).
3. **Traces.** What every act leaves, by fixed rules per act kind (§6).
4. **Knowledge.** What the player gathers: traces they find, things people tell them, things they see. Each item is a belief with a source and an age (spec 39).
5. **Deduction.** The player joins partial facts (§7) and acts on the conclusion (§10).

**The ground rule ([0067](../decisions/0067-stories-are-found-never-written.md)): truth first, traces from the truth, never clues first.** Nothing is placed to make a case fair. A case is fair or cold depending on what its events happened to leave (§8).

## 4. Living lore: hooks

Hooks are the crime-flavoured **seeds** of [story-blocks](story-blocks.md) §3. The same machinery carries old flames, promises and lost siblings for every other kind of story.

**A hook** is a past fact that ties two real pops and can become a **motive** (I want something from B, or want to hurt B), **leverage** (B has to help me, or keep quiet) or **trust** (I can recruit B).

| Hook | Example from a lifepath | Residue it leaves (derived when someone looks) |
| --- | --- | --- |
| Debt | Borrowed $4,000 from the pharmacist in 2019 | An IOU in the pharmacist's desk; the borrower's wife knows |
| Grudge | Was laid off by the plant manager | A union letter; coworkers remember the shouting |
| Secret | An affair with a neighbour | Letters in a shoebox; a hotel receipt |
| Favour owed | The sheriff's deputy covered a DUI for him | A missing page in the station's log for that night |
| Old crime | Embezzled from the diner | A ledger with a gap; the former cook suspected |
| Rivalry | Lost the council election to her | A newspaper clipping; party members on both sides |
| Shared guilt | Both were there when the boy drowned | Both hold the memory; neither has told |

- **Generated with the lifepath, as a pure function of the seed.** A hook always names a second person from an existing census slot (content tools §4.2: no new people), and lists its residue by kind and place. Today's lifepath events have neither ([MY-17](../research/procedural-mysteries.md)). Hooks fill the gap, and they are the "ties to other pops" content tools §4.1 lists for later.
- **Residue is placed in real places.** Documents go in real furniture of real dwellings and workplaces (spec 51 interiors); memories go in real people. All of it is derived when someone looks (opens the drawer, asks the wife) and kept once seen (0048 rule 2).
- **Hooks are dormant until pressure.** Pop AI reads them as modifiers on its choices. The pressures come from the simulation: scarcity, fear, a cordon, an office falling vacant, the police gone.

**An example of lore coming alive:** the guard waved the pharmacist's van through the cordon.
- **The lore:** he has owed the pharmacist since 2019.
- **The simulation:** the cordon and the antibiotic shortage.
- **The AI:** the debt plus the money tipped his choice.
- **The trace:** the IOU in the pharmacist's desk, the van's entry missing from the gate log, and a soldier who saw the van at 2 a.m.

Nobody wrote that story.

- **Most hooks never matter, which is right.** Lore is only worth generating when it can connect to play ([LB-14](../research/library-of-babel-and-sacred-geometry.md)). A hook is cheap data until it wakes.
- **Default budget:** 0–3 hooks per adult (an estimate), derived and never stored. A woken hook is kept on the pop's record.

## 5. Secret acts and schemes: where crimes come from

**A secret act** is an action the actor wants unknown: a theft, hiding a bite, a bribe, a meeting, falsifying a record, a killing, moving a body, burning a letter.
- **The AI chooses it when the gain beats the expected cost.** It judges that cost from its *beliefs*: who could see or hear (light and noise at that place and hour), how likely it is to be caught, and how strong the law still is.
- **So crime rises, for reasons you can read, when the lights go out and the police are gone.** Nothing is rolled from a crime table.

**A scheme** is a secret plan with several people and roles: organiser, inside man, lookout, buyer, protector.
- **Members are recruited along ties and hooks:** trust, debt, fear.
- **Dwarf Fortress precedent:** villains corrupt officials, with success set by social skill and weak values ([MY-4](../research/procedural-mysteries.md)).

**Default scheme kinds**, grown from apocalypse pressures (none scripted; each is an AI goal that forms when its conditions hold):

| Scheme | Forms when | Typical traces |
| --- | --- | --- |
| Diverting controlled stock (medicine, food, fuel) | Shortage, plus someone with access and a buyer | Ledger gap; dispensing-book entries that don't match; night trips |
| Selling places on an aid or evacuation list | A list exists and one office controls it; leaders broker aid ([PKC-74](../research/playing-knowing-commanding-moving.md)) | Names on the list with no right to be there; payments; envy |
| Hiding an outbreak | An official fears panic or blame (the outbreak grammar's cover-up) | Hospital records changed; staff told to keep quiet; a public version that contradicts the deaths |
| Smuggling across a cordon | A cordon, a price difference, a guard with a hook | Gate log gaps; goods where they shouldn't be |
| Removing a rival before a succession | An office opens, two candidates | A death with a convenient timing; who gained |
| Revenge | A grudge hook, plus the chance when order breaks down | The old hook's residue points straight back |
| Hoarding and price fixing | Scarcity and a few sellers | Stock counts versus what was offered |

**Cover-ups add clues.** After a secret act, an actor who believes they may be found out acts to hide it:
- cleans up or burns papers;
- lies, or threatens a witness;
- changes a ledger, or moves a body.

Each of these is a new secret act with its own traces ([MY-23](../research/procedural-mysteries.md)). Following a criminal leads to the people who protect them, and that is how a theft becomes a conspiracy.

**Scale:**
- Schemes run among records and in places with detail: the player's attention and the world's story spots ([story-explorer](story-explorer.md) §3).
- Cold places have simple crimes as rates. When the player opens one, it is derived under the rule in §11.

## 6. Traces: what acts leave

| Kind | Made by | Where it sits | Fades | What it can reveal |
| --- | --- | --- | --- | --- |
| **Physical** | Wounds (type and region, spec 12), blood, corpses (rot phase dates the death; flies), broken locks, footprints (only in mud or snow), moved items, fuel used | At the place, on bodies | Hours (prints, blood) to months (skeleton) | Weapon, time, entry, how many people |
| **Ledger gap** | Every transfer is exact (spec 32) | In owners' accounts. The owner's *count* is a belief, so the gap appears when someone counts | Never, until recounted | That something left, roughly when, and who had access |
| **Documents** | Notes, letters, IOUs, lists, receipts, logs (gate, radio, dispensing, duty rosters), marked maps, diaries | In furniture, pockets, offices | Until burned, wet or lost | Partial facts with blurred names (§7) |
| **Witness memories** | Perception: anyone above the sound notice threshold or with line of sight and light ([MY-22](../research/procedural-mysteries.md)) | In people | Confidence decays; details drift; a dead witness takes it with them | Who, when, a coat colour at 30 m in low light |
| **Statements** | What people say when asked: true, partial or a lie (a `Tell` whose claim differs from the teller's own belief) | In conversation, then in your notes | As the teller changes their story | Contradictions are the clue |
| **Infrastructure** | Phone records, CCTV, the radio log, while power and staff last | In institutions | Gone when the grid or the staff go | Exact times and places |
| **Absence** | Something that should have happened and didn't | In the gap | — | A door opened with no sound of forcing points to a key. Holmes' dog that did not bark ("Silver Blaze") |

**A diary is a pop's own beliefs written down each day,** true or false. A diarist who was lied to writes the lie. A diarist who is the liar writes something else. Every pop has a diary page rendered on demand, but only diarists leave a physical one in the world ([story-blocks](story-blocks.md) §6).

## 7. How note A and note B add up: partial references

Every fact a trace carries is a sentence with blanks: **who** did **what** to **whom**, **where**, **when**, **why**. Traces blur some of the blanks:

| Blank | Sharp → blurred |
| --- | --- |
| Person | full name → initials → nickname or role ("the doctor") → "he" → nothing |
| Place | address → name → "the usual place" → the district |
| Time | exact → "Thursday" → "after the blackout" |
| Thing | "the shelter's evacuation list" → "the list" → "it" |

**How much a trace is blurred:**
- **writers** blur by their caution trait and what is at stake;
- **witnesses** blur by perception: distance, light, noise, fear.

**The player joins sentences on their shared blanks.** An example:
- **Note found on A:** "Thursday, the usual place. Bring the list."
- **B's diary:** "Saw R. at the chapel again Thursday night."
- **The guard roster:** B on the chapel gate on Thursday nights.
- **A neighbour:** "a car without lights stopped by the chapel around two".
- **What they add up to:** A is Rafael, so R. is A. The usual place is the chapel. Only one list exists in town, the shelter's evacuation list.

Each join can be checked, because each sentence is a projection of the same real meeting. This is *The Case of the Golden Idol*'s fill-in-the-blanks, except that our words are found in the world rather than handed out ([MY-8](../research/procedural-mysteries.md)).

## 8. Solvable without authoring

- **The Three Clue Rule as a check, not a script** ([MY-1](../research/procedural-mysteries.md), [MY-20](../research/procedural-mysteries.md)). When a secret act happens, count each key fact's independent trace routes: physical, document, witness, ledger, infrastructure.
  - **Three or more:** a **fair case**.
  - **Fewer:** still real, but a **cold case**, ranked low and kept for experts.
- **Cases change over time.** Decay and cover-ups remove routes, and new witnesses add them. Time pressure is real: prints fade in hours, as in Shadows of Doubt.
- **Finding is physical; reading takes skill** ([MY-21](../research/procedural-mysteries.md)):
  - you find what is in the drawer if you open it;
  - a medic reads a wound as a bite or a blade, while others see "a wound";
  - a bookkeeper sees the ledger doesn't balance;
  - no dice decide whether a clue exists.
- **A solver in the tests.** Given every trace that exists at tick t, can each key fact be pinned down by joins? Its answer is the case's **solvability**, which the librarian uses to rank cases. The literature checks solvability by playtest ([MY-12](../research/procedural-mysteries.md)); ours can also be measured.

## 9. How a case reaches the player

- **You see it happen** (the user's example). Your attention makes the place detailed. You see the act and tail the criminal, who can notice you, since perception works both ways. They meet someone at a real scheme meeting, and now you are inside the conspiracy.
- **You find a trace:** flies over a lot, a stock count that is short, a strange note in a house you are searching, a rumour.
- **Someone asks you.** A pop with a low-confidence suspicion (a belief) asks for help, because they want the truth or want revenge.
- **The librarian:** "the case" joins the story explorer's patterns. It matches a secret act with surviving traces and enough solvability, and shows as a pin with a question mark.
- **Stations give tools** (0058). A police or militia office opens the case tools: request records (ledgers, logs), interrogate, get a warrant, arrest. Without a station you can look, ask, trade and threaten. Interrogation that names the next link is Dwarf Fortress's justice loop ([MY-5](../research/procedural-mysteries.md)).

## 10. Deduction and the verdict

- **The board is the story explorer's Board, as a case board.** It holds:
  - evidence cards (traces with source and age);
  - hypotheses (the blank sentences, filled as you believe);
  - strings (joins).

  It never makes the leap for you ([LB-47](../research/library-of-babel-and-sacred-geometry.md)).
- **There is no oracle in the world.** You act on your conclusion: accuse, arrest, confront, tell the mayor, broadcast. The world responds by its rules. A wrong accusation hurts an innocent and warns the guilty.
- **The reveal.** When a case ends in the world (a confession, a conviction, a death, the truth surfacing), or when your pop dies, a **truth replay** puts your board next to the truth, fact by fact: who, what, where, when, why. Spec 25 already lists the after-death truth replay as later work; it is the same machinery.
- **An optional "detective" setting:** confirmation in threes, as *Obra Dinn* does, so you can play cases as puzzles without being able to guess ([MY-7](../research/procedural-mysteries.md)).

## 11. Zoom: everything has a reason

- **Derive on look, keep what was seen** (0048 rule 2). Interiors, drawers, papers and memories are pure functions of (seed, place, hooks, events). They are derived when someone looks and stored once seen.
- **Constrained derivation for cold places:**
  - the cold model decides the outcome: a burglary on day 12, the culprit chosen by rank, whether it was solved;
  - the derivation decides only the evidence, and it must agree with the outcome. If the culprit was arrested in the world, the evidence the investigator used exists.
  - Derivation never changes who or what (the [story-explorer](story-explorer.md) rule "the past at the detail it was lived").
- **Every trace knows why it is there.** In free mode, clicking a trace shows the event or hook that put it there. That makes the logic between everything something you can inspect: a debug lens for us and a pleasure for the observer.
- **Physical checks hold everywhere:**
  - travel times between places;
  - the light at that hour (sun formulas);
  - the weather (footprints need mud or snow);
  - power (CCTV only while the grid is up);
  - keys (who had access, from rosters).

## 12. Build order (simulation first)

1. **The first case inside Tier 0: who took the fuel** ([MY-24](../research/procedural-mysteries.md)). The run already has a fuel crunch near day 10 and a secretive trait ([MY-18](../research/procedural-mysteries.md)). Generalise spec 24 rule 7 from "hide a bite" to "hide an act". A pop under pressure may take fuel at night if it believes nobody will hear. Traces:
   - the ledger gap;
   - the generator's hours;
   - footsteps above the notice threshold;
   - a false statement when asked.

   Tested headless: `zs cases` prints each case with its trace routes and solvability.
2. **Traces as things:** documents and marks with decay; witness memories from perception (specs 18, 19); statements that can lie (spec 39 `Tell`).
3. **Hooks in the lifepath data:** a second person, a kind and residue (a data and generator change), plus `zs hooks <pop>`.
4. **The case pattern for the librarian,** and the solvability solver.
5. **Schemes:** several actors recruited along ties and hooks, after records and offices exist.
6. **The case board in the 2D window:** a small panel; the explorer's Board later.

## 13. Defaults taken (no open items)

- **Crimes come only from AI choices under pressure.** No scripted crimes and no murderer generator ([MY-6](../research/procedural-mysteries.md)).
- **Hooks:** 0–3 per adult, derived, never stored until woken.
- **The fair-case threshold:** three independent trace routes.
- **Confirmation:**
  - **knowledge mode:** nothing until the case ends;
  - **"detective" setting:** threes;
  - **free mode:** the truth is visible, with a per-case "hide the truth" switch (lowering your detachment is always allowed).
- **Diaries:** pops with a writing habit (a trait) write their beliefs daily.
- **The outbreak origin is the first great case.** Its grammar is in inhabitants; its traces follow §6.

## Research

Notes that feed this design ([0054](../decisions/0054-research-notes-findings-and-links.md)); cite their findings by ID.

- [story-building-blocks](../research/story-building-blocks.md): Story building blocks: how games and story theory build variety from parts
- [lore-made-and-generated-worlds](../research/lore-made-and-generated-worlds.md): Lore in made and generated worlds: Project Zomboid, other games, and tools outside games
