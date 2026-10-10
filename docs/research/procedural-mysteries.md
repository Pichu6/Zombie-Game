# Procedural mysteries: cases that come from a simulation

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [inhabitants](../design/inhabitants.md) (fact, memory, evidence and account; background residue; outbreak origin), [content tools](../design/content-tools.md) §4.1, [spec 24](../tier0/specs/24-pop-ai.md), [spec 25](../tier0/specs/25-chronicle.md), [spec 39](../tier0/specs/39-orders-and-belief.md), and the design drafts [cases-and-living-lore](../design/cases-and-living-lore.md) and [story-explorer](../design/story-explorer.md). **Evidence:**
- **Search summaries only:** the Alexandrian, Pelgrane Press and GUMSHOE pages, the Dwarf Fortress wiki (Intrigue, Villain, Justice, 0.47.01 release), Steam forum posts on Shadows of Doubt (player reports, not the developer), Obra Dinn and Golden Idol coverage, and the abstracts of Cook 2020, Mysterious Murder 2019 and WikiMystery and DATA Agent.
- **Read in our docs and data:** inhabitants, body trauma, specs 24, 25, 26 and 32, `data/gen/lifepath.toml`.
- **Not read:** any full paper; the Shadows of Doubt developer's own description of murder generation (not found).

**IDs:** MY.


The user asked for mysteries and cases solved from many sources: a note on one character, another on a second, a crime seen and the criminal followed into a conspiracy. The user also asked how background lore could become procedural during the simulation, so that everything has a reason when zoomed in.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| MY-1 | The Alexandrian's Three Clue Rule: for any conclusion players must reach, plant at least three clues. Players miss some and misread others, so the redundancy is the safety net. The inverse also holds: three clues in hand almost always lead to at least one conclusion | reported | thealexandrian.net/?p=7985 |
| MY-2 | Node-based scenario design: each node (a place, person or event) is also a conclusion, and every node the players know should give at least three leads to nodes they have not visited. The rule is about what is reachable, not what they will find | reported | thealexandrian.net node design pages |
| MY-3 | GUMSHOE: a character who uses a relevant ability in a scene containing a core clue finds it, with no roll, which moves play from finding clues to interpreting them. Pelgrane's advice is to start from the villains' scheme and map the trail of clues from the trigger | reported | en.wikipedia.org/wiki/Gumshoe_System; pelgranepress.com GUMSHOE rules summary |
| MY-4 | Dwarf Fortress's villains release (0.47.01, January 2020) gave villains goals and let them subvert others into embezzlement, sabotage, kidnapping or assassination. Organisations send agents to corrupt residents, with success depending on both sides' social skills; criminal gangs bribe officials to embezzle; dwarves with weak values can fall into lasting corruption, which exposes them to law and blackmail. The release was cut short, and much fortress-side villainy was left for later | reported | dwarffortresswiki.org Intrigue, Villain, DF2014:Release_information/0.47.01 |
| MY-5 | In Dwarf Fortress the justice screen keeps the evidence gathered from witnesses, interrogation and surveillance as leads to the next person to question. Interrogating a traitor's handler can name their master; visitors with bad intent show nothing until interrogated and may use aliases | reported | dwarffortresswiki.org Intrigue, Justice |
| MY-6 | Shadows of Doubt killers, as players describe them: four calling-card types (an anagram note, lipstick, a business card, a toy car), each usually with the killer's prints. Killers keep killing until caught. One player says the killers have no relationship-based motives, though corporate killers target coworkers. Anecdotal and pre-1.0 | reported | steamcommunity.com app 986130 discussions |
| MY-7 | *Return of the Obra Dinn* (2018) confirms fates only in sets of three (the last six in pairs) to stop guessing. A fate is identity, manner of death and killer; faces stay blurred until the memory that identifies them has been seen | reported | en.wikipedia.org/wiki/Return_of_the_Obra_Dinn; filmstories.co.uk on the rule of three |
| MY-8 | *The Case of the Golden Idol* (2022): fill-in-the-blank sentences take colour-coded words collected from the scene. A correct section locks; one or two wrong gives a yellow "close" warning. Early prototypes that built whole sentences from phrases tested worse than "where does each phrase fit?" | reported | gamedeveloper.com "Pursuing the Aha! moment" |
| MY-9 | Michael Cook, "Generative Forensics" (arXiv 2004.01768, 2020): information games are mysteries where the player gathers knowledge to understand an event or system. A simulation generates the mystery, which eases the spoiler problem and makes replay possible. Prototypes: *Nothing Beside Remains*, *Condition Unknown* | reported | arxiv.org/pdf/2004.01768 (abstract) |
| MY-10 | *Mysterious Murder* (IEEE CIG 2019): a Ceptre-style rule simulation with believable agents produces causally connected actions ending in a murder, and the player works out who could have done it | reported | opus.cloud1.lib.uts.edu.au handle 10453/141401; github.com/corinnaj/mysterious-murder |
| MY-11 | WikiMystery and DATA Agent (Barros, Liapis, Togelius and others, FDG 2018) pick suspects and clue traits from open data by evolutionary search. A playtest with 30 people found the mysteries straightforward to solve | reported | antoniosliapis.com papers |
| MY-12 | We found no paper that proves a generated mystery has a unique solution; the work above checks solvability by playtest | estimate | MY-9..MY-11 |
| MY-13 | Our design already separates the four layers a mystery needs: world fact, personal memory (with confidence and errors), evidence (objects, wounds, corpses, records, places, testimonies) and circulating account (rumour or official version). Every background step "leaves verifiable residue" (contacts, access, goods, debts, injuries, an open question). The outbreak origin is fixed but hidden, with the grammar cause → first incident → witnesses → cover-up → public version | sourced | [inhabitants](../design/inhabitants.md), Stories and relationships |
| MY-14 | Our only secret act today: an infected pop with the secretive trait hides the wound and says "just a cut". It is the one false belief in Tier 0; the chronicle tags it "hidden bite" | sourced | [spec 24](../tier0/specs/24-pop-ai.md) rule 7; [spec 25](../tier0/specs/25-chronicle.md) |
| MY-15 | Planned physical traces: corpses rot in phases and attract flies from over 100 m, whose buzzing reveals hidden bodies; smell travels through the room graph | sourced | [body trauma](../design/body-trauma.md) |
| MY-16 | The stock ledger is exact: no hidden mint, and every owner's tallies reconcile with its balance. A theft is a transfer that leaves a gap in someone's count | sourced | [spec 32](../tier0/specs/32-stocks-goods-and-money.md) |
| MY-17 | Lifepath events today are text with an optional skill ("was laid off", "learned to hunt"). They name no second person and leave no residue in the world | sourced | `data/gen/lifepath.toml` |
| MY-18 | The Tier 0 run already has the pressure a first case needs: a fuel crunch near day 10, and a stranger with a 0.5 chance of a hidden bite who is secretive or honest | sourced | [spec 26](../tier0/specs/26-run.md) |
| MY-19 | **Proposal:** truth first, clues from the truth. Every clue is a projection of a real event, so joining clues can never reach a world that did not happen. Cook's simulation-made mysteries (MY-9) and GUMSHOE's "start from the scheme" (MY-3) point the same way | derived | MY-3, MY-9 |
| MY-20 | **Proposal:** the Three Clue Rule as a generation check. Count each key fact's independent trace routes (physical, document, witness, ledger, infrastructure) when the act happens. A case below three is still real but ranked as a cold case | derived | MY-1, MY-12 |
| MY-21 | **Proposal:** GUMSHOE's split for us: finding is physical (you open the drawer, you are at the scene), and reading depends on skill (a medic reads a wound as a bite or a blade; others see "a wound") | estimate | MY-3 |
| MY-22 | **Proposal:** witnesses come from physics. Whoever was above the notice threshold of a sound (spec 18) or in line of sight with enough light (spec 19) when an act happened holds a memory of it; nobody else does | derived | spec 18, spec 19; HST-139 |
| MY-23 | **Proposal:** a cover-up is itself a secret act with its own traces, so hiding a crime adds clues instead of ending the case | estimate | MY-13 (cover-up in the outbreak grammar) |
| MY-24 | **Proposal:** the first case can be built inside Tier 0: who took the fuel during the day-10 crunch. Traces: the ledger gap, the generator's run log, the noise at night, a lie in a statement | estimate | MY-14, MY-16, MY-18 |

## What we take and what we don't

**Take:**
- the Three Clue Rule as a check, not a script (MY-20);
- GUMSHOE's split between finding and reading (MY-21);
- Dwarf Fortress's corruption through social skill and weak values, and interrogation that names the next link (MY-4, MY-5);
- Obra Dinn's confirmation in threes, as an option (MY-7);
- Golden Idol's blanks, filled with words found in the world (MY-8);
- Shadows of Doubt's board that never makes the leap for you (LB-47).

**Don't take:**
- **A murderer generator.** Shadows of Doubt's killers have calling cards rather than motives (MY-6). Ours commit crimes only when their own needs, ties and pressures make it worth it to them.
- **Clues placed for fairness.** A clue that no event caused would break "found, never written" ([story-explorer](../design/story-explorer.md) §2).

## Sources

- **Design craft:** [Three Clue Rule](https://thealexandrian.net/?p=7985), [node design](https://thealexandrian.net/creations/misc/node-design/node-design2.html), [GUMSHOE (Wikipedia)](https://en.wikipedia.org/wiki/Gumshoe_System), [GUMSHOE rules summary](https://pelgranepress.com/2017/09/29/gumshoe-rules-summary/).
- **Games:** [DF Intrigue](https://dwarffortresswiki.org/index.php/Intrigue), [DF Villain](https://dwarffortresswiki.org/index.php/Villain), [DF 0.47.01](https://www.dwarffortresswiki.org/index.php/DF2014:Release_information/0.47.01), [Shadows of Doubt Steam discussions](https://steamcommunity.com/app/986130/discussions/0/3882723164279391344), [Obra Dinn](https://en.wikipedia.org/wiki/Return_of_the_Obra_Dinn), [Golden Idol design](https://www.gamedeveloper.com/design/case-of-the-golden-idol).
- **Research:** [Generative Forensics](https://arxiv.org/pdf/2004.01768), [Mysterious Murder](https://opus.cloud1.lib.uts.edu.au/handle/10453/141401), [WikiMystery](https://antoniosliapis.com/papers/murder_mystery_generation_from_open_data.pdf), [DATA Agent](https://antoniosliapis.com/papers/data_agent.pdf), [ClueGen](https://ojs.aaai.org/index.php/AIIDE/article/view/12896).
