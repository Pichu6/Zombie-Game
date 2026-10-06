# Content tools

The internal generators that fill the world: names, people (pops), items and props, buildings and towns. They exist so the world is large, varied and consistent without hand-writing every person and room, and so every run can be different.

Labels: **Decided**, **Proposal**, **Implemented**, **Validated** (see CLAUDE.md). Everything here is a Proposal unless marked. Requested by the user on 2026-10-06; it builds on [world simulation](world-simulation.md) §3 (human generator, lifepath) and §12 (settlements, culture kits), [macro layer](macro-layer.md) §6 (countries and cultures in setup) and decisions [0016](../decisions/0016-sandbox-setup-and-layer-flow.md), [0017](../decisions/0017-slow-endgame-and-generation-on-the-go.md) and [0022](../decisions/0022-macro-layer-intent.md).

## 1. Rules for every generator

1. **Pure function of a seed.** Output = f(world seed, id, data). No hidden counters: one person, street or room can be regenerated alone, in any order, and comes out the same. This is what lets the lazy census hold everyone on Earth at 0 bytes (world simulation §3).
2. **Part of the simulation core, headless.** Generators run at world creation and during play (generation on the go, 0017), so they live in C++ next to the sim (`engine/gen`, linked to `engine/sim`) and are tested without a window. Editors and previews are views over them (stage 5 sandbox and later).
3. **Data, not code.** Cultures, jobs, business types, item templates and building kits are TOML packs under `data/gen/`. Adding a culture or a job is a data change.
4. **Saved at creation.** Generated content that matters is stored with the world when first made (world simulation §12), so a generator or data update never changes an existing save. Goldens pin the generator output to catch accidental changes.
5. **Realistic distributions, never stereotypes.** Packs carry shares (names, household sizes, appearance) from public statistics, written as estimates where no source fits. Culture never sets a person's job, skills, morals or ability; those come from age, education and the job table, shared by everyone.
6. **Generation order: places, then names, then people, then their pasts** (Decided by the user, 2026-10-06). Anything a later step names must already exist: if a background says "cook at Pete's Diner" or "went to Maple Elementary School", that business or school is in the town directory. Places outside the generated area are real cities from the culture packs. Today the order is: town name and streets → businesses and institutions (with their owners' names) → households and people → lifepaths.
7. **One-way links to the sim.** A generator produces records; the sim reads them. Generators never read live sim state (except the outbreak history for damage, later).

## 2. The tool set

| Tool | Produces | Data | Stage | State |
| --- | --- | --- | --- | --- |
| **Name generator** | People (culture rules: surname order, two surnames, gendered surnames), streets, businesses and institutions, towns, provinces, countries | culture packs, `businesses.toml` | Tier 0 | Implemented |
| **Pop creator** (human generator, Tier 0 subset) | Households and people: name, sex, age, heritage, nationality, birthplace, when they moved here, home and address, education, job, workplace, skills tags, skin tone, hair, eyes, body seed, and a lifepath (§4.1) | culture packs, `jobs.toml`, `lifepath.toml`, a population mix | Tier 0 | Implemented |
| **Town directory** | Town name, streets, the businesses and institutions people work at | culture packs, `businesses.toml` | Tier 0 | Implemented |
| **Full human generator** | Body from real anthropometry (the census already draws one from the body seed), health conditions with medicine clocks, personality, face genes shared by families, clothes, possessions and keys, ties, lifepath sampled backwards with real rates | packs above + NHANES/ANSUR, condition rates, ILO/O*NET jobs | Tier 1 (records, world simulation §3) | Proposal |
| **Item and prop creator** | Variants of an item from a template (material, size, brand, quality, wear), brands from the name generator; a linter that checks every item's mass, volume and properties against bands and that salvage conserves mass | `data/tier0/items.toml` model (spec 15), templates | When items pass ~50 (first at stage 9 content pass) | Proposal |
| **Interior generator** | Furniture and loot placed in an existing room shell from the room type and the household record (a nurse's flat, a hunter's garage); every placement follows the building's ledger (0017) | room kits per culture and era | Tier 1 (the first generated houses) | Proposal |
| **Building generator** | Plot → massing → rooms (interior first, every room reachable) → doors and windows → facade by grammar | culture kits at five levels (world simulation §12) | Tier 1 to Tier 2 | Proposal |
| **City brush and settlements** | Street pattern and plots for a district; town placement by the suitability function | culture kits, real data (world simulation §3, §12) | Tier 2+ (world creation and Sandbox) | Proposal |
| **Country and province generator** | Names (implemented), borders from the region graph, government and economy numbers within validator bands, culture shares | country presets (macro layer §6) | Macro layer | Names Implemented, rest Proposal |

Asset meshes and textures are not generated here: art follows [art direction](art-direction.md) and the engine stack decision ([0002](../decisions/0002-engine-stack.md)). The building and interior generators place modules that art provides.

## 3. Culture packs

One file per culture in `data/gen/cultures/<id>.toml`. Tier 0 ships six: `american` (European heritage), `african_american`, `mexican`, `korean`, `nigerian`, `polish`.

| Field | Meaning |
| --- | --- |
| `country`, `nationality` | Where the culture's own people are born, and their nationality |
| `name_order` | `given_family` or `family_given` (written in the town's order when living abroad) |
| `surnames` | `single`, `paternal_maternal` (two surnames; people born in a town abroad show only the first), `gendered_ski` (Kowalski → Kowalska) |
| `given_m`, `given_f`, `family` | Names most common first; rank r is drawn with weight 1 / (r + 2) |
| `places` | Real towns: birthplaces, and the training set for new place names |
| `town_forms`, `province_forms`, `country_forms` | How a new name is dressed: `{root} Springs`, `{root}-do`, `Republic of {root}` |
| `street_format`, `street_names`, `street_kinds` | `{name} {kind}`, `{kind} {name}` or `{name}-{kind}` |
| `skin`, `hair`, `eyes` | Appearance shares: skin on the 10-point Monk Skin Tone scale |

New place names come from an order-2 letter chain trained on `places`, so they sound like the culture (`Newpordville`, `Santa Uruada`, `Gimhaegu-dong`, `Tarszamoszow`) and never copy a real town. Tier 0 packs are ASCII only.

Later levels of the same pack (world simulation §12): household size and family structure, trust in authority and reaction to rules (macro layer §6.2), building style per era, interiors and loot.

## 4. Pop creator

Input: a population mix (`data/gen/populations/<id>.toml`): the town's culture, people count, on-map homes, household size shares (US ACS 2022), intermarriage and same-sex couple rates, and groups with a share, the share of founding adults born abroad and the chance a person born here gets a given name from the town's culture.

Per household, in order:

1. **Group and size.** One group by share; size from the household shares (5+ becomes 5-7).
2. **Head and partner.** Couples by size; a partner sometimes comes from another group (intermarriage); immigrants marry immigrants of their group more often.
3. **Moves.** A first-generation adult came to the country at 16-45; the household has lived at its address for a drawn number of years that fits every adult.
4. **Children and relatives.** Children are 18-45 years younger than the youngest parent and mostly under 25; a household with no possible parent gets adult relatives. Children born before the family emigrated are born abroad; children younger than the household's years here are born in the town.
5. **Names.** The father's family name (else the head's) goes to the children; a wife takes it at the culture's rate; Mexican children carry the mother's surname as their second one; no two people in a household share a given name.
6. **Appearance.** Founders draw from their culture; children blend their parents' skin tone with some spread and inherit hair and eye colour from a parent; hair greys with age.
7. **Work.** Under 5: at home; 5-17: elementary or high school; 18-22: college for some; adults draw a job by employment share among the jobs their age allows, with education at least the job's level; retirement from 62; some are out of work; a past job for some over 35. The workplace is a business from the town directory.

Output per pop: see the table in §2. `zs_gen town` prints a dossier with the lifepath, for example:

```
Jose Gomez Lopez, 55, male | American (naturalized)
  Cook at Taqueria Ximena for 20 years. Education: high school.
  Born in Chihuahua, Mexico; moved here at 27. Lives with wife Lily at 199 Maple Drive (house_b), for 22 years.
  Skin 5/10, brown hair, brown eyes | skills: cooking, knives, people
    1976-1985  school student in Chihuahua, Mexico
    1985-1989  high school student in Chihuahua, Mexico
    1989-1998  cashier in Chihuahua, Mexico
    1998-2004  cook in Newport, United States; 1998 came to United States from Mexico; 2003 married Lily
    2004-2006  out of work; 2004 moved to New Wington
    2006-now   cook at Taqueria Ximena
```

### 4.1 Lifepath (Implemented, 2026-10-06)

Requested by the user: a background built from the years lived, like TTRPG lifepath systems (Traveller terms, Twilight: 2000). Rules in `data/gen/lifepath.toml`.

- **Terms.** A pop's life is a list of terms, oldest first, that add up exactly to their age: school (elementary from 5, high school from 14), study (trade school, college or graduate school by education level), military service for some adults, jobs and spells out of work, then the present (today's job, retirement, school, or "small child").
- **Built back from today.** The present is fixed first (today's job and how long they have had it, or when they retired); earlier jobs are filled backwards to the end of their studies in terms of 2-9 years. An earlier term often keeps the same job (careers stick), and some are gaps. The pop's job and education from the pop creator are the anchors, so the past never contradicts the present.
- **Places follow where they lived.** Each term is cut where they moved. Abroad until they emigrated; adults grew up in their birth city; then the household's city before it came here; then the town. In the town, a term names a business or school from the directory; elsewhere it names the real city.
- **Shared facts.** A couple's marriage is one date for both (before their oldest child, after an immigrant partner arrived); every child's birth appears in both parents' lives; a family that emigrated together arrives together.
- **Events.** Each term can draw events from the table (broke an arm, car accident, surgery, lost a parent, laid off, learned to hunt, first aid course). Some events leave a skill, and jobs held for 3 or more years add their skills: the past is where the skills come from.
- **Later:** real rates per age and country (education, service, divorce, injuries), event effects on the body (old fractures, surgery scars) and the mind, ties to other pops met in a shared workplace or school (one shared event remembered by both, world simulation §3), and prose biographies made lazily from the terms.

Skills are tags for now; the know-how model ([activities](activities.md), [inhabitants](inhabitants.md)) turns them into practice levels later.

### 4.2 Mentions are links (Proposal, from the user's idea on 2026-10-06)

Any person a text names (a spouse, a business owner, a coworker, a teacher) is a link: clicking the name focuses the camera on that person, or opens their dossier when they are far away.

- **No new people.** A mention always points at a census slot that already exists from the world seed (world simulation §3, lazy census). Clicking it never invents anyone; it reads that slot and promotes it to a record if needed. The world population is fixed at creation, so the cascade is finite by construction.
- **Lazy, one step at a time.** Opening a person generates their dossier and lifepath; the people *they* mention stay unopened links (ids) until someone clicks them. Cost grows only with what the player actually looks at.
- **Through the character's eyes.** In individual play a link shows only what the character knows (world simulation §13 inspector); the god view shows everything.
- **Needed in the generator:** mentions carry person ids, not just text; business owners, coworkers and schoolmates are picked from existing census slots (in the town or a neighbouring area record), so "Pete's Diner" is owned by a real Pete.

## 5. How it plugs into the sim

- **Draws:** `zs::gen::Keyed`, a stateless Philox draw on (seed, id, field, k). Field numbers are append only (`engine/gen/src/fields.h`).
- **Census:** `zs_gen census --seed N` writes the same table format as `data/tier0/census.toml` (plus `job` and `culture`, which the stage 7 loader ignores). Tier 0 keeps its current hand-checked table; switching the census to read the generator directly (person = f(world seed, slot)) is a stage 9 or Tier 1 change for the census owner, and changes the stage 7 goldens.
- **Pop AI:** jobs and skills tags are ready for the AI's job choice (spec 24) once it reads them.
- **Setup:** the culture and mix files are what the setup screens (macro layer §6) edit: auto, preset or custom.

## 6. Commands

```
zs_gen town   [--seed N] [--mix FILE]     # town, streets, businesses, then every pop's dossier
zs_gen census [--seed N] [--mix FILE]     # pops as a census table
zs_gen names  --culture ID [--kind person|street|town|province|country] [--count N] [--seed N]
zs_gen cultures
```

## 7. Tests (Implemented, 2026-10-06)

`zs_gen_tests`: every pack loads; lifepaths over 30 seeds (terms are contiguous and end today, every "at" place is in the town directory and every "in" place is a real city or the town, the present term matches the job and its years, a couple has one marriage year); culture name rules (gendered and double surnames, family-first order); new place names never copy a real one; a town is a pure function of its seed; household consistency over 30 seeds (ages, parents at least 18 years older, children carry a parent's surname, job education and workplace, immigrants' move ages); every group and 25+ jobs appear over 40 towns; census export has every person and three survivor slots; a golden hash of seed 1, same on GCC 13 and Clang 18.

Not validated: whether the people read as real and varied to the user. That needs the user to read some dossiers.

## 8. Open

- Which real region the Tier 0 town is modelled on (0011 left it open); the mix file assumes a mid-size US town.
- Whether generated pops replace the hand-made census table now or at stage 9.
