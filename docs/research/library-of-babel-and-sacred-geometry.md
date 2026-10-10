# The Library of Babel, the Babel Image Archives and "sacred geometry": what a library of everything teaches a simulation

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [spec 28](../tier0/specs/28-pop-levels.md), [spec 06](../tier0/specs/06-save-replay.md), [spec 30](../tier0/specs/30-world-grid.md), [macro layer](../design/macro-layer.md), [zoom continuum](../design/zoom-continuum.md), [content tools](../design/content-tools.md), [0061](../decisions/0061-the-world-runs-without-the-player.md), [0062](../decisions/0062-the-game-loop-attention-possession-and-late-starts.md). **Evidence:**
- **Read (primary, through a page summariser):** libraryofbabel.info `About.html` and `theory4.html`; babelia.libraryofbabel.info `about.html`; the GitHub README of `librarianofbabel/libraryofbabel.info-algo`; h3geo.org "Indexing" highlights; Wikipedia's article on Borges' story.
- **Search summaries only (LB-45..LB-50, added later the same day for the [story explorer](../design/story-explorer.md) draft):** Google Earth's 2017 relaunch coverage, the Dwarf Fortress Legends pages, Shadows of Doubt, Watch Dogs: Legion's Census, *Bad News*, hyperlink cinema. **Also search summaries only:** Ryan's dissertation, Winnow, Compton's oatmeal essay, the Dwarf Fortress rejection pages, Keinert 2015, Roberts' R-sequences, GBT, Gosper islands, IGEO7, Falbo, Markowsky and Bartlett.
- **Read in our code:** `engine/gen/include/zs/gen/keyed.h`, `engine/sim/src/pop.cpp` (`CohortPermutation`), spec 28, spec 30, spec 06 and the macro layer design.
- **Not read:** Basile's book *Tar for Mortar*; the original Gibson and Lucas 1982 paper; Compton's original essay.

**IDs:** LB.


The user asked whether the concept behind libraryofbabel.info and its image twin could spark something new for the simulation. The site's claim is that everything is already written and drawn, and you only need the address. The user then added "sacred geometry", because Borges' library is built of hexagons.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| LB-1 | libraryofbabel.info is Jonathan Basile's version of Borges' story "The Library of Babel" (1941) and his essay "The Total Library" (1939). It uses 29 symbols (26 lower-case letters, space, comma, period) and pages of 3,200 characters, and claims every combination of 1,312,000 characters (one 410-page book) | sourced | libraryofbabel.info/About.html |
| LB-2 | Nothing is stored. Each page is computed on request from its location, and the same location gives the same page "in perpetuity" | sourced | libraryofbabel.info/About.html, theory4.html |
| LB-3 | Search is the generator run backwards. Basile's first version stored random books and searched them one by one. A Halton-sequence version failed because its divisions lost information, so it could not be inverted. The final version mixes modular arithmetic and bit shifts, and is invertible: text in, location out | sourced | libraryofbabel.info/theory4.html |
| LB-4 | The values involved are about 5,000 digits (about 16,000 bits) | sourced | libraryofbabel.info/theory4.html |
| LB-5 | A page has 29^3,200 ≈ 10^4,680 possibilities, which is 15,546 bits. A one-to-one map cannot give shorter addresses on average than its contents (pigeonhole), so the address is as long as the page. The library compresses nothing; it relabels | derived | LB-1, LB-4 |
| LB-6 | Basile's published algorithm (the image site's version): a linear congruential step `x = (a·x + c) mod m` with m larger than the number of items and Hull–Dobell full-period conditions, then Mersenne-Twister-style xorshift "tempering". Each step is undone in reverse: tempering backwards, then multiplying by a's modular inverse (extended Euclid). Arbitrary precision (GMP, Boost). He added tempering because "the LCG on its own did not create sufficiently random-seeming results" | sourced | github.com/librarianofbabel/libraryofbabel.info-algo README |
| LB-7 | Basile on searching: search finds only what you already know or wrote; finding what was never written needs browsing. Readable pages are a grain of sand on a shoreline, and rarity does not make a page meaningful. Visitors move from joy to melancholy and then, he hopes, to clarity | sourced | libraryofbabel.info/theory4.html |
| LB-8 | The Babel Image Archives (babelia.libraryofbabel.info) hold every 416 × 640 image in 4,096 colours (16 levels per channel). A location number, almost 1,000,000 digits long, seeds the generator. Visitors browse a "Universal Slide Show" of random images or search | sourced | babelia.libraryofbabel.info/about.html; 16 levels per channel reported by qz.com |
| LB-9 | 4,096^266,240 = 10^961,755 images (266,240 pixels × 12 bits = 3,194,880 bits), which matches the site's count and its million-digit locations | derived | LB-8 |
| LB-10 | Basile on images: noise is overwhelmingly likely, and meaning, if it is there, needs a "magic eye" to see | sourced | babelia.libraryofbabel.info/about.html |
| LB-11 | Borges' library: hexagonal rooms, books of 410 pages × 40 lines × about 80 letters, 25 symbols. "Purifiers" destroy books they judge nonsense while seeking the Crimson Hexagon of magical books. Some believe in the "Man of the Book", who has read a perfect index, and travel to find him | reported | en.wikipedia.org/wiki/The_Library_of_Babel |
| LB-12 | Kate Compton's "10,000 bowls of oatmeal": a million outputs that are each unique mathematically do not matter if players cannot tell them apart. Every oat can sit in a new place and orientation, and the player still just sees oatmeal | reported | emshort.blog 2016-09-21; Wikipedia "Procedural generation"; Game Developer "Practical procedural generation for everyone" |
| LB-13 | On *Spore*, Compton went through as many generated planets as she could and whitelisted the seeds of good ones; she judged this faster than writing a proper algorithm | reported | Wikipedia "Procedural generation" |
| LB-14 | Emily Short's test: generated content matters when it is tied to mechanics. Otherwise it is decoration and players learn to look past it | reported | emshort.blog 2016-09-21 |
| LB-15 | Dwarf Fortress generates a world, checks it against "rejection" minimums (mountains, volcanoes, biomes, room for features) and re-rolls it if it fails; a log can record each reason. Setting the minimums to zero ends rejections and leaves the result to chance | reported | dwarffortresswiki.org v0.31 and v0.34 world generation pages, 40d:World_rejection |
| LB-16 | James Ryan's dissertation *Curating Simulated Storyworlds* (UC Santa Cruz, 2018) argues that emergent narrative needs curation: **overgenerate and curate**. A simulation produces masses of banal events, and a **story sifter** picks out the storyful ones, like a prospector sieving sand for gold. He described this as "story recognition" in 2015 | reported | emshort.blog review series 2019; arXiv 2304.08293; wbs.cdn.uib.no wiki |
| LB-17 | Ryan's *Talk of the Town* simulates an American town from 1839 to 1979 before play | reported | emshort.blog 2019-05-28 |
| LB-18 | Winnow (Kreminski, Dickinson, Mateas, AIIDE 2021) is a language for sifting patterns that run **incrementally**: a story can be detected while it is still happening, and partial matches can be shown to the user before they complete. Earlier sifters (Felt) only saw a story after it had played out | reported | ojs.aaai.org AIIDE 17(1) 156-163 |
| LB-19 | Leong, Porteous and Thangarajah (IJCAI 2022) build changing character-relationship networks from a simulation's event trace and score them against a user's requested story arc with dynamic time warping | reported | ijcai.org/proceedings/2022/0686.pdf |
| LB-20 | Our keyed draws are already Babel's rule: a draw is a pure function of (seed, id, field, k) through Philox4x32-10, with no counters, so any person can be regenerated alone from the world seed and their id | sourced | `engine/gen/include/zs/gen/keyed.h`; [spec 02](../tier0/specs/02-draw.md) |
| LB-21 | We already have Babel's search too. `CohortPermutation` is a 6-round Feistel network with cycle walking over a cohort, and `inverse(rank)` returns the member with a given rank. Today only `forward` is called (`pop_registry.cpp`) | sourced | `engine/sim/src/pop.cpp`, `engine/sim/include/zs/pop.h`; [spec 28](../tier0/specs/28-pop-levels.md) rule 2 |
| LB-22 | Spec 28 makes a pop's life random-access: one draw `u(pop_id, "death")` becomes a death time through the inverse cumulative hazard, so any pop's state at any time is computed without stepping the others. Born pops have invertible ids (founder slot plus the path of child numbers) | sourced | [spec 28](../tier0/specs/28-pop-levels.md) rules 1 and table |
| LB-23 | Our macro layer already says "events are detected, never injected": every headline comes from a real event record, and each country has a Chronicle of significant events | sourced | [macro layer](../design/macro-layer.md) §2 and the events section |
| LB-24 | Our save header already holds the binary hash, data hash, seed, settings and tick (spec 06, header table, rule 16) | sourced | [spec 06](../tier0/specs/06-save-replay.md) |
| LB-25 | A 64-bit world seed reaches at most 2^64 ≈ 1.8 × 10^19 worlds per build and settings. Our generator does not hold every world; it picks a tiny, structured set of shelves, and what it cannot reach no player will ever see | derived | LB-5, LB-20 |
| LB-26 | The Babel move needs a map with no lost information. A simulation step loses information: two states can lead to the same next state (two different pops each die, leaving the same room). So a step cannot be inverted, just as Basile's Halton version could not, and rollback still needs snapshots or logs | derived | LB-3; RA rollback notes |
| LB-27 | "Sacred geometry" is a mystical label for real shapes: the Seed and Flower of Life are circles on a hexagonal lattice (one centre and six around), plus the Platonic solids and the golden ratio. Most of the aesthetic and nature claims fail: people do not prefer, or even recognise, golden rectangles (Markowsky 1992), and nautilus shells measure about 1.24–1.43 (Falbo), or 1.31 across 80 Smithsonian shells (Bartlett 2019), not 1.618 | reported | Markowsky, *College Mathematics Journal* 23(1) 1992; sciencenews.org on Falbo; Bartlett et al., *Nexus Network Journal* 2019 |
| LB-28 | Our planet grid is already that geometry: a regular icosahedron built from golden-ratio coordinates, subdivided, with 10n² + 2 cells (12 pentagons, the rest hexagons). Cell centres nest (every centre at n is a centre at 2n), but the hexagonal cells do not tile their parents, which is why HY-50 needs explicit child area shares | sourced | [spec 30](../tier0/specs/30-world-grid.md) rule 1; [zoom continuum](../design/zoom-continuum.md) open questions |
| LB-29 | H3 (aperture 7: one centre child and six around, the Seed of Life pattern) states that "hexagons do not cleanly subdivide into seven finer hexagons". Containment is exact in the index but only approximate on the ground; exact uses need a point-in-polygon check | sourced | h3geo.org/docs/highlights/indexing |
| LB-30 | Generalized Balanced Ternary (Gibson and Lucas, 1982) addresses hexagons with seven digits: a centre and the six points of a hexagon around it; its base is a matrix | reported | en.wikipedia.org/wiki/Generalized_balanced_ternary |
| LB-31 | The Gosper island splits exactly into seven smaller copies of itself (linear scale √7) and tiles the plane. Exact seven-way nesting is possible only with this fractal border | reported | en.wikipedia.org/wiki/Gosper_curve; Bridges 2017 paper on Gosper map projections |
| LB-32 | IGEO7 with Z7 integer indexing is a pure aperture-7 equal-area hexagonal global grid in DGGRID (FOSS4G Europe 2026 talk) | reported | talks.osgeo.org/foss4g-europe-2026/talk/HSFXR3 |
| LB-33 | Spherical Fibonacci Mapping (Keinert, Innmann, Sänger, Stamminger; ACM TOG 34(6) 193, 2015): an inverse that returns the nearest point of a spherical Fibonacci set of any size in constant time, with no tables | reported | history.siggraph.org; lgdv.tf.fau.de publication page |
| LB-34 | Martin Roberts' R-sequences (2018): point n is `(α0 + n·α) mod 1`, with α from the golden ratio in 1-D and the plastic constant (about 1.3247) in 2-D; in his comparison they spread more evenly than Sobol and Halton | reported | extremelearning.com.au "The unreasonable effectiveness of quasirandom sequences" |
| LB-35 | The golden ratio is the irrational number hardest to approximate with fractions, so stepping by it never lines points up. This is the true part of sunflower phyllotaxis (Vogel's 137.5° golden angle), and the same reason LB-33 and LB-34 work | reported | standard result; extremelearning.com.au; Vogel 1979, *Mathematical Biosciences* |
| LB-36 | **Proposal:** any trait the game must be able to *find* among lazy pops is assigned by rank (`rank < k` through `CohortPermutation`), never by an independent per-pop draw. Then "every nurse in this cohort" is `inverse(0) … inverse(k − 1)`: k inversions instead of generating the whole cohort, and the count agrees exactly with the macro number | derived | LB-3, LB-21 |
| LB-37 | **Proposal:** a librarian for the director. Sifting patterns over the event log (sequences, not single events) run incrementally and turn partial matches into leads ("a story is forming in Lyon") that feed the headline feed, the Chronicle, pinned windows and the observer camera. It still detects and never injects | estimate | LB-16, LB-18, LB-23; 0062 |
| LB-38 | **Proposal:** call numbers. For a world nobody plays, the save header (build and data hashes, seed, settings, tick) is the full address of a moment. Print it as a short code anyone can open on the same build; a played world adds its input log. The hashes must be in the code because any rule change reshelves the library. Opening is not instant as on Babel: it simulates to the tick, or starts from the nearest daily save | estimate | LB-2, LB-24; 0061 |
| LB-39 | **Proposal:** Babel layer or chronicle layer. Each field is marked either a pure function of (seed, place, time), recomputed never stored, or stepped history. Anything stepped needs a reason | estimate | LB-20, LB-22; SE "derive, don't store"; improvements item 12 |
| LB-40 | **Proposal:** exact nesting for things that are counted. Smooth fields keep hexagon parents with child area shares (HY-50). People, buildings and ledger quantities need every fine cell owned by exactly one parent: the centre child plus 3 of its 6 edge children by a fixed direction rule (4 per parent, matching our aperture 4), with written rules for the 12 pentagons and the icosahedron's face seams. Parents get ragged borders, as Gosper islands do, and sums are exact by construction | derived | LB-28..LB-31 |
| LB-41 | **Proposal:** an oatmeal test for generators. Over N seeds, a test builds each town's one-line signature from things a player notices (biggest employer, river or coast, age skew, culture mix, size band) and fails if too many collide. Each signature item must change play | estimate | LB-12, LB-14 |
| LB-42 | **Proposal:** golden-ratio scattering for small things inside a cell (wrecks, caches, masts, trees): item i sits at `(α0 + i·α) mod 1`, a pure function of i. Adding items never moves old ones, which Poisson-disk sampling cannot promise. Jitter each with a keyed draw so it does not read as a grid. On the sphere, a Fibonacci set with Keinert's inverse answers "nearest of N" in constant time | estimate | LB-33..LB-35 |
| LB-43 | **Proposal:** "find my family". A survivor's relatives abroad are lazy pops with invertible ids, and their state on day t is a pure function that agrees with the macro totals. The game can answer "alive? last seen where?" through the survivor's beliefs (radio, rumour), never the truth in knowledge mode. A player who arrives meets her with exactly that past. This is Borges' Crimson Hexagon as a quest: the one book that matters exists and is consistent, and finding it is the game | estimate | LB-11, LB-22, LB-36 |
| LB-44 | **Proposal:** "a world where X" is rejection search. It is not inversion, because a step loses information (LB-26): generate, check, re-roll, as Dwarf Fortress does, or keep curated seed lists as Compton did. Cheap for generator-level presets (a check costs seconds); too slow for long histories | estimate | LB-13, LB-15, LB-26 |
| LB-45 | Google Earth's 2017 relaunch: Voyager guided tours (about ten places each, around 40–50 tours at launch, with partners such as the BBC and NASA); "I'm Feeling Lucky" (a die icon) draws from a curated database of 20,000 places "worth exploring"; on landing a knowledge card shows images and stories and can be shared. Google said it reflects people who explore "without any particular destination in mind" | reported | siliconangle.com 2017-04-18; 9to5google.com 2017-04-18; inverse.com article 30439 |
| LB-46 | Dwarf Fortress Legends mode browses maps, each civilization's history and the life of any figure who lived or died; sites and regions keep their own event histories. Legends Browser rebuilds it in a web browser with every object a link. A world with 1,000 or more years of history can export an XML dump of up to about 1 GB | reported | dwarffortresswiki.org Legend, v0.34:Legends, DF2014:Utilities, XML dump |
| LB-47 | Shadows of Doubt generates a city of hundreds of citizens per save, each with a home, a job and a schedule of about 4–10 trips a day, planned in a pause of up to 10–15 s before each day. Evidence (receipts, addresses, fingerprints) is pinned to a corkboard and joined with red string; the board updates but "won't make logical jumps for you"; fingerprints and blood fade within hours | reported | en.wikipedia.org/wiki/Shadows_of_Doubt; colepowered.com DevBlog #4 and #8 |
| LB-48 | Watch Dogs: Legion's Census is a relational database: a passer-by's identity is filled in when the player profiles them (job from uniform, hours from time of day, neighbourhood from income), then kept persistent; social profiles drive schedules including meetings with friends, relations and adversaries | reported | news.ubisoft.com "The tools that built London" 2021; gdcvault.com/play/1027018 (abstract) |
| LB-49 | *Bad News* (Ryan, Summerville, Samuel; CHI 2016 extended abstract): the player finds the next of kin of a dead resident in a simulated 1979 small town of several hundred characters who hold subjective knowledge of each other, with a live actor playing each resident; IndieCade 2016 Audience Choice | reported | eis.ucsc.edu/papers/ryanEtAl_BadNewsCHI2016.pdf; santacruztechbeat.com 2016-10-21 |
| LB-50 | "Hyperlink cinema" (Alissa Quart, *Film Comment*, 2005; popularised by Roger Ebert): separate stories whose connections are slowly revealed. Examples: Altman's *Short Cuts*, Soderbergh's *Traffic* and Iñárritu's *Babel* (2006), whose four stories in different countries follow from one event | reported | en.wikipedia.org/wiki/Hyperlink_cinema |

## 1. Answer in one page

**What the sites are.** One fixed, invertible function maps every address to every possible page (LB-1..LB-6) or image (LB-8, LB-9). Nothing is stored. Reading computes; searching runs the function backwards. The address is as long as the content (LB-5), so the library compresses nothing. It is a relabelling of all possible strings, and nearly all of them are noise (LB-7, LB-10).

**Spark 1: we already built a Babel, and half of its search.**
- The keyed draws (LB-20), random-access lives (LB-22) and invertible pop ids are Babel's "content is a function of its address".
- `CohortPermutation::inverse` (LB-21) is Babel's search function, and nothing calls it yet.
- The new rule is LB-36: assign by rank any trait we want to search. Then the director can ask "show me every pilot in France", a survivor's radio can ask "is there a doctor within 20 km", and the possession picker can list candidates. It works across billions of lazy pops with no storage, and the counts match the macro numbers exactly.

**Spark 2: the real problem of an 8-billion-pop world is the Library of Babel's problem.**
- Our stories will happen; the library holds them. But in an 8-billion-pop world they are lost in the noise, and the player can only search for what they already know (LB-7).
- The field that solved this is story sifting: overgenerate, then curate (LB-16).
- The modern form spots a story while it is still forming (LB-18). That fits our "detected, never injected" rule exactly (LB-23).
- This is the biggest idea here: a **librarian** that finds stories as they form for the director, the observer camera and the headline feed (LB-37). It turns "watching changes nothing" (0061) from a constraint into the observer's best feature.

**Spark 3: call numbers.**
- A world runs without the player and is deterministic, so a moment in it has an address (LB-38). Our save header is already most of one (LB-24).
- Players could share "seed 41, day 37, 14:02, Lyon" as a code, and anyone opens the same moment and sees the same thing.
- One honest difference from Babel: our pages must be walked to (simulated), not computed directly, so daily saves serve as shelf markers.
- Basile's "same place forever" holds only because his algorithm never changes. Our call numbers must carry the build and data hashes, or old codes point to the wrong shelf.

**Smaller sparks:**
- **Find my family** (LB-43): Borges' Crimson Hexagon as a quest for a real, consistent person.
- **Babel layer or chronicle layer** for every field (LB-39).
- **The oatmeal test** for the generators (LB-41).
- **Rejection search** for scenario presets (LB-44).

**What the Babel idea cannot do for us:**
- **Run a simulation backwards** (LB-26). A step merges states, just as Basile's Halton attempt lost information. Rollback keeps its snapshots and command logs.
- **Make "every possible world exists" a feature.** It is oatmeal at the limit (LB-12): every output unique, none distinguishable.

## 2. "Sacred geometry": the true part is already in our planet

Take away the mysticism (LB-27) and three real facts remain. Each one already touches our design:

| Shape | The real maths | Where we have it, or could use it |
| --- | --- | --- |
| **Icosahedron and golden ratio** | A regular icosahedron's vertices are golden-ratio coordinates | Our planet grid is built exactly this way (LB-28), like H3, DGGRID and geodesic domes |
| **Seed of Life** (one circle, six around) | The aperture-7 parent and children of H3 and GBT (LB-29, LB-30). Hexagons cannot split exactly into smaller hexagons; only the fractal Gosper island can (LB-31) | Answers the open HY-50 question: shares for smooth fields; a partition with ragged parents for anything counted (LB-40) |
| **Golden angle** (sunflower) | Stepping by the most irrational number spreads points evenly and never lines them up (LB-35) | Addressable, extendable scattering inside cells, and "nearest of N" on the globe in constant time (LB-42) |

The fun fact for the user: the "sacred geometry" in this project already works. It is the icosahedral grid under the whole planet, and the golden ratio is in its first line of construction.

## 3. What we take, ranked

| # | Proposal | Spec | Cost | When |
| --- | --- | --- | --- | --- |
| 1 | Searchable traits assigned by rank; call `inverse` for queries (LB-36) | 28, content tools | Small | With the next pop-traits work; the function exists |
| 2 | The librarian: incremental sifting patterns over the event log, feeding leads to the director, observer camera, headlines and Chronicle (LB-37) | macro layer, 0062, new spec | Medium | When the event log has cause chains at macro scale |
| 3 | Call numbers: the save header as a shareable code plus "open at tick" (LB-38) | 06 | Small | After save format 5 (header fields land there) |
| 4 | Each field marked Babel or chronicle (LB-39) | 27, 28 | Small | With the chunk spec |
| 5 | Exact partition for counted things, with shares kept for smooth fields (LB-40) | 30, 27 | Medium | With HY-59 or the chunk work |
| 6 | Oatmeal signature test for towns and countries (LB-41) | content tools, 52 | Small | Any time; generator side |
| 7 | Golden-ratio scattering inside cells (LB-42) | zoom continuum T2 | Small | When T2 amplification is built |
| 8 | Find my family, through beliefs (LB-43) | 28, 39 | Small after 1 | A layer-1 hook for the demo flow |
| 9 | Rejection search for generator presets and curated seed lists (LB-44) | sandbox parameters | Small | With presets |

**Rejected:**
- Reversible simulation steps (LB-26).
- "Every world exists" as a selling point (LB-12).
- Hex cells at street level: buildings are rectangular, so spec 50's 8 directions stay.
- The golden ratio as an aesthetic rule for art or layout (LB-27).

## 4. Open

Defaults are chosen here and can be overridden:

- **Call numbers:** a player feature, not only a debug tool. Default: yes for observer and sandbox worlds; played worlds share a replay instead.
- **The librarian's patterns:** data files, like culture packs, so modders can write their own. Default: yes, with a small built-in set (last stand, reunion, betrayal, a town that holds and then falls, brothers on opposite sides).
- **HY-50:** partition or shares. Default: shares for elevation and other smooth fields; the partition for anything counted.
- **Not confirmed:** whether Gibson and Lucas's GBT digits map onto Gosper sub-islands. This note's link between them is a synthesis.

## 5. Sources

- **The sites:** [libraryofbabel.info About](https://libraryofbabel.info/About.html), [theory4](https://libraryofbabel.info/theory4.html), [Babel Image Archives about](https://babelia.libraryofbabel.info/about.html), [algorithm README](https://github.com/librarianofbabel/libraryofbabel.info-algo), [Wikipedia: the website](https://en.wikipedia.org/wiki/The_Library_of_Babel_(website)), [Wikipedia: the story](https://en.wikipedia.org/wiki/The_Library_of_Babel), [Quartz 2015](https://qz.com/446122/jorge-luis-borgess-library-of-babel-has-been-digitized-enjoy).
- **Procedural generation and curation:** [Emily Short on oatmeal](https://emshort.blog/2016/09/21/bowls-of-oatmeal-and-text-generation/), [Wikipedia: Procedural generation](https://en.wikipedia.org/wiki/Procedural_generation), [Ryan dissertation review](https://emshort.blog/2019/05/28/curating-simulated-storyworlds-james-ryan-ch-6f/), [Winnow](https://ojs.aaai.org/index.php/AIIDE/article/view/18903), [IJCAI 2022](https://www.ijcai.org/proceedings/2022/0686.pdf), [DF world rejection](https://dwarffortresswiki.org/index.php/40d:World_rejection), [DF v0.34 advanced world generation](https://dwarffortresswiki.org/index.php/v0.34:Advanced_world_generation).
- **Exploring simulated worlds (LB-45..LB-50):** [SiliconANGLE on Google Earth 2017](https://siliconangle.com/2017/04/18/google-earth-gets-makeover-new-tours-im-feeling-lucky-option/), [9to5Google](https://9to5google.com/2017/04/18/google-earth-chrome-voyager-feeling-lucky/), [DF Legends](https://dwarffortresswiki.org/index.php/DF2014:Legend), [Shadows of Doubt](https://en.wikipedia.org/wiki/Shadows_of_Doubt), [Shadows of Doubt DevBlog #4](https://colepowered.com/?p=33958), [Ubisoft: the tools that built London](https://news.ubisoft.com/en-us/article/4po3S9Pwp1YcgBmGPmQxAh/watch-dogs-legion-the-tools-that-built-london), [GDC: Census](https://gdcvault.com/play/1027018/Census-The-Systemic-Backbone-Behind), [Bad News, CHI 2016](https://eis.ucsc.edu/papers/ryanEtAl_BadNewsCHI2016.pdf), [Hyperlink cinema](https://en.wikipedia.org/wiki/Hyperlink_cinema).
- **Geometry:** [H3 indexing](https://h3geo.org/docs/highlights/indexing), [GBT](https://en.wikipedia.org/wiki/Generalized_balanced_ternary), [Gosper curve](https://en.wikipedia.org/wiki/Gosper_curve), [Bridges 2017](https://archive.bridgesmathart.org/2017/bridges2017-237.html), [IGEO7 talk](https://talks.osgeo.org/foss4g-europe-2026/talk/HSFXR3/), [Spherical Fibonacci Mapping](https://www.lgdv.tf.fau.de/publications/spherical-fibonacci-mapping/), [R-sequences](https://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/), [Science News on Falbo](https://www.sciencenews.org/?p=26206), [Markowsky PDF](https://webarchive.lib.cuhk.edu.hk/20210329125916mp_/http://www.math.cuhk.edu.hk/~pschan/uged1533/markowsky.pdf), [Bartlett 2019](https://link.springer.com/10.1007/s00004-018-0419-3).
