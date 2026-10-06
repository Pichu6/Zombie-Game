# World systems for factions: territory, subjects, expansion, conquest, collapse

- **Date:** 2026-10-06. **Status:** research note, **not a decision**. Nothing here is Decided, Implemented or Validated. It feeds [0036](../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md) (factions hold areas of control, expand, shrink, become subjects or conquerors) and the loop of [0035](../decisions/0035-realistic-world-numbers-and-the-pop-nation-loop.md) (state -> pop -> state). It sits beside [world numbers](../design/world-numbers.md), [macro layer](../design/macro-layer.md) and [war in other games](war-in-other-games.md).
- **Method and limits.** WebFetch was **blocked** by the network egress proxy (tested on stellaris.paradoxwikis.com; the earlier notes report the same for other domains). **Every number below comes from WebSearch result summaries**, tied to the URL the search returned; no page was read in full. Game-wiki and forum numbers are version-dependent (patches change them) and are marked low where they come from players or guides. "Not found" means the searches did not return it. Equations in section 2 are **our own proposals** unless said otherwise.
- **Confidence:** H = institutional or primary source in the summary; M = secondary summary of a primary source or an official dev diary; L = guide, forum, aggregator or study guide: check before use.
- **Slavery.** Treated here only as a historical and economic fact that some games model (row in section 1.9). Nothing in this note recommends modelling it ([0036](../decisions/0036-worlds-start-with-nations-and-everything-follows-logic.md) keeps it open).

## 1. Numbers and rules

### 1.1 Territorial units and what a unit holds

| Quantity or rule | Value or range | Source URL | Confidence |
| --- | --- | --- | --- |
| Victoria 3: a **State** is the main political-geographic unit; pops live there and buildings are built there; always controlled by one country; borders are dynamic. A **State Region** has static borders and holds 1 or several States (a "split state" when two countries share a region) | rule | https://forum.paradoxplaza.com/forum/developer-diary/victoria-3-dev-diary-16-states.1491897/latest | M |
| Stellaris: a **Sector** is an administrative region under a governor; the capital sector reaches 4 jumps in every direction; colonies outside it sit in a "Frontier Sector"; sectors are made free at any frontier colony; systems cannot be added or removed by hand, only by moving borders | 4 jumps | https://admin-forum.paradoxplaza.com/forum/developer-diary/stellaris-dev-diary-147-an-update-on-sectors-designations.1175704/page-5 (search summary also cites steamcommunity.com/app/281990/discussions/0/3825286438457369883) | M |
| Stellaris: past a limit of directly controlled planets, "empire size" penalties hit influence and empire-wide income; the limit depends on government and technology; governors reduce crime and empire size and add a flat output bonus | rule, limit not found | same thread as above | L |
| HOI4: map = provinces grouped into **states**; a state holds factories, resources and population; ownership, control (occupation) and core are separate. See 1.4 | rule | https://admin-forum.paradoxplaza.com/forum/developer-diary/hoi4-dev-diary-resistance-and-compliance.1240349 | M |
| CK3: a **County** holds provinces that can each take a holding (castle, city, temple); the de jure county capital has 4 building slots, others 3. **County control**: each point below 100 gives -1% tax and -0.5% levies | 100 control cap; -1% tax, -0.5% levy per missing point | https://www.gamewatcher.com/crusader-kings-3-holdings-guide , https://guildorder.com/games/ck3/wiki/vassal-management-and-council | L |
| Common pattern: a control unit is a **bundle of pops plus buildings with one controller and one owner** (V3 state, CK3 county, HOI4 state). The game-wide stats of a nation are sums over these units | derived | (pattern across rows above) | derived |

### 1.2 Occupation, ownership, claims

| Quantity or rule | Value or range | Source URL | Confidence |
| --- | --- | --- | --- |
| HOI4 (Resistance and Compliance, patch 1.9 "La Resistance"): occupied states have **resistance** (grows or decays toward a target set by state development, whether the core owner still exists, laws) and **compliance** (rises with time, unlocks factories and resources). Garrisons drain manpower and equipment in proportion to resistance and compliance. High resistance slows movement, raises supply use and attrition | rule | https://eip.gg/hoi4/news/hearts-of-iron-iv-patch-1-9-new-intelligence-system-la-resistance-dlc/ , https://admin-forum.paradoxplaza.com/forum/developer-diary/hoi4-dev-diary-resistance-and-compliance.1240349 | M |
| HOI4 occupation law: the harshest law lowers resistance by 75% | 75% | https://gamerempire.net/hearts-of-iron-4-how-to-deal-with-resistance/ | L |
| Stellaris war: wargoals and **claims** express territorial ambition; war exhaustion runs 0 to 100%, rises from enemy occupation of planets and starbases, combat losses and passive attrition; at 100% a side can be forced into status quo peace | 0 to 100% | https://admin-forum.paradoxplaza.com/forum/developer-diary/stellaris-dev-diary-93-war-peace-and-claims.1054054 | M |
| CK3: de jure titles, claims and occupation decide who may fight for a county; not retrieved in detail | not found | (search returned no numbers) | n/a |

### 1.3 Subjects in games

| Quantity or rule | Value or range | Source URL | Confidence |
| --- | --- | --- | --- |
| Stellaris types: Vassal, Subsidiary, Tributary, Protectorate, Bulwark and others. Terms are percentages of the **subject's** production; tributaries, subsidiaries and prospectoria always give the overlord at least **30%** of basic resources (energy, minerals, food) | min 30% | https://admin-forum.paradoxplaza.com/forum/developer-diary/stellaris-dev-diary-247-new-ways-to-rule.1516799/page-11 | M |
| Stellaris loyalty: set by the agreement terms plus ethic compatibility; example term "Independent Diplomacy" gives +5 loyalty and +0.5 a month | +5, +0.5/month | https://devtrackers.gg/stellaris/p/c203c43b-stellaris-dev-diary-247-new-ways-to-rule | M |
| Stellaris protectorates get a large research bonus until they catch up with the overlord | rule | same | M |
| HOI4 autonomy ladder (democratic, non-aligned, communist): Independent, Dominion, Colony, Puppet, Integrated Puppet; (fascist): Satellite, Reichsprotektorat, Reichskommissariat. Integrated Puppet threshold 1250 autonomy points; lowering by 500 (50 political power) reaches Integrated Puppet, a further 1000 (300 political power) annexes. Lend-lease lowers autonomy 1 per 100 production points | 1250; 500; 1000 | https://gamerempire.net/hearts-of-iron-4-how-to-annex-puppets/ , https://gamerempire.net/hearts-of-iron-4-how-to-lower-autonomy/ | L |
| Victoria 3 subject types: Protectorate (autonomous, no fully independent foreign policy), Puppet (no diplomatic autonomy, pays part of income, part of overlord market), Dominion and Territory (colonial), Tributary (only for unrecognised powers, pays part of income, has own subjects). No percentages found | rule; % not found | https://admin-forum.paradoxplaza.com/forum/developer-diary/victoria-3-dev-diary-20-diplomatic-actions.1495234 | M |
| Victoria 3 **Liberty Desire**: how pressing autonomy is to the subject's ruling class; high values make AI subjects "Break Free" or seek self-reliance; influenced by lobbies and relative prestige of subject versus overlord | rule | https://admin-forum.paradoxplaza.com/forum/developer-diary/victoria-3-dev-diary-111-subject-improvements.1657476 | M |
| CK3 vassal contract: feudal base levy contribution **16.6% / 25% / 33.3% / 50%** by obligation level; high levy obligation costs -10 opinion (was -15); crown authority multiplies vassal levy and tax **100% / 110% / 120% / 130%** (autonomous, limited, high, absolute); a vassal can ask better terms only for an even trade or with a hook | as listed | https://steamcommunity.com/sharedfiles/filedetails/changelog/2222896774?p=9 , https://steamcommunity.com/app/1158310/discussions/0/3771239049942905802 (changelog summaries; version-dependent) | L |

### 1.4 Subjects in history

| Quantity or rule | Value or range | Source URL | Confidence |
| --- | --- | --- | --- |
| China, Ming/Qing tribute: formalised under the early Ming; mission frequency by state: Korea 4 a year (Ming rule) or annual (Qing), Annam once every 2 years, Siam every 3, Laos and Burma every 10; Ryukyu sent 57 missions 1372 to 1398 (about 2 a year) | as listed | https://en.wikipedia.org/wiki/Tributary_system_of_China | M |
| China: the emperor's return gifts and trade permits were worth **more** than the tribute, so tribute was a net transfer to the subject; tributaries were "virtually autonomous" and could not expect Chinese military help when invaded | rule | https://en.wikipedia.org/wiki/Tributary_system_of_China , https://qinghistorysg.com/research-on-qing-history/qing-tributary-system/ | M (the "system" itself is debated: see https://resolve.cambridge.org/core/services/aop-cambridge-core/content/view/EB785B19BB6D304E68409942BB4DDAEA/S1557466013033536a.pdf/mapping-china-and-the-question-of-a-china-centered-tributary-system.pdf) |
| Neo-Assyria: vassals bound by treaty and oath; more productive states paid more; policy toward western states was to keep them as vassals unless compelling reasons; revolts by small units were frequent and "vexing"; 67 campaigns in the Levant in two centuries; Tiglath-pileser III (744 to 727 BC) annexed vassals into provinces (Arpad, Damascus 743 to 732 BC) after Urartu's rise caused defections | 67 campaigns / 200 years | https://en.wikipedia.org/wiki/Pax_Assyriaca , https://www.andrews.edu/library/car/cardigital/Periodicals/AUSS/1997-1/1997-1-02.pdf | M |
| Rome, client kings: Augustus used Bosporus, Cappadocia, Lycia, Mauretania, Nabataea as clients; annexation of some was judged necessary and the Julio-Claudians and Flavians continued turning kingdoms into provinces; new clients mostly when legions needed support or respite | rule; rate not found | https://etheses.durham.ac.uk/id/eprint/10140/ , https://www.cambridge.org/core/journals/journal-of-roman-studies/article/abs/provincial-monarchs-as-an-eastern-arcanum-imperii-client-kingship-the-augustan-revolution-and-the-flavians/6A0194009C718F23D9EAF91189E1DF63 | M |
| Ottoman tributaries: Moldavia, Wallachia, Transylvania, Ragusa, Crimean Khanate, Cossack Hetmanate; Moldavia a tributary for over three centuries; tribute 2,000 gold coins a year in 1451 to 1457; relations "did not follow a single pattern" | 2,000 gold / year (1451 to 1457) | https://en.wikipedia.org/wiki/Moldavian%E2%80%93Ottoman_Wars , https://www.bib.irb.hr/668286 | M |
| British India: subsidiary alliance (subject cedes external relations, hosts and pays for British troops) then **Doctrine of Lapse** annexed a state with no natural male heir; after 1858 annexation was renounced and paramountcy over the remaining princely states kept | rule; counts and shares not found | https://en.wikipedia.org/wiki/Political_integration_of_India , https://vajiramandravi.com/upsc-exam/doctrine-of-lapse (as returned: https://vajiramandravi.com/quest-upsc-notes/doctrine-of-lapse) | L |
| Typical tribute or revenue share across history | **not found** (only the single Ottoman coin figure above) | n/a | n/a |
| Typical subject rebellion rate and duration of subject status | **not found** as a number; qualitatively frequent in Assyria, rare in the Chinese system (no obligation to defend, little to resent) | sources above | L |

### 1.5 Expansion and what limits it

| Quantity or rule | Value or range | Source URL | Confidence |
| --- | --- | --- | --- |
| Boulding's **loss-of-strength gradient** (1962): the military power a state can bring to bear falls with distance; forward bases flatten it; Boulding later argued modern transport reduces it, another scholar argues it remains relevant. No single decay constant given in the summaries | rule, no constant | https://en.wikipedia.org/wiki/Loss-of-strength_gradient , https://www.militarystrategymagazine.com/article/risk-distance/ | M |
| Related: Tobler's first law and distance decay | rule | https://en.wikipedia.org/wiki/Loss-of-strength_gradient | M |
| Lanchester: square law (aimed fire, loss rate proportional to the enemy's size): power proportional to N squared; to beat a force 3 times as large a fighter must be 9 times as effective. Linear law for "ancient" one-on-one duels and area fire | exponent 2 or 1 | https://en.wikipedia.org/wiki/Lanchester%27s_laws | M |
| Historical force ratios are **poor predictors** of battle results (Dupuy Institute critique of Lanchester on history) | rule | https://dupuyinstitute.org/2018/05/02/the-lanchester-equations-and-historical-warfare | M |
| Medieval sieges: most strongholds that fell did so in under 2 weeks, often within 24 hours; some lasted months or years | < 14 days for most that fell | https://dhc.westpoint.edu/project/medieval-sieges-project/ (West Point Medieval Sieges Project; summary text) | L |
| HOI4 terrain: mountains +40% attrition, -66% combat width, -40% attack, +100% movement time; large river -50% attack, small river -25%; combat width caps how many divisions fight | as listed | https://eip.gg/hoi4/guides/best-combat-width/ , https://forum.paradoxplaza.com/forum/threads/hearts-of-iron-iv-28th-development-diary-weather-terrain.885969/post-20061548 | L |
| HOI4 / V3 / Stellaris logistics (supply range from the capital, naval capacity upkeep) | see [war in other games](war-in-other-games.md) | (that note) | M |
| Turchin et al. 2013: war, diffusing cavalry/chariot technology and rugged terrain explain 65% of variance in where large states arose (16% without military technology) | 65% | https://pmc.ncbi.nlm.nih.gov/articles/PMC3799307 | M |
| Carneiro circumscription: bounded agricultural land (valley, coast, desert) makes war losers stay and submit, so states form | rule | https://en.wikipedia.org/wiki/Circumscription_theory | M |
| Taagepera: state size grew in three phases separated by technology jumps; empire duration did **not** grow over five millennia | rule | https://cambridge.org/core/books/more-people-fewer-states/growthdecline-patterns-and-durations-of-empires/BE64D7F6A2C0D1CE600879E35A1E280C , https://resolve.cambridge.org/core/journals/social-science-history/article/size-and-duration-of-empires-growthdecline-curves-600-bc-to-600-ad/86549F8696AE51653D1B3D917338F655 | M |
| Natural expansion by population pressure and settlement (rate of frontier advance, colony founding) | **not found** here; see [settlement note](world-numbers-settlement.md) and [capacity note](world-numbers-capacity.md) | n/a | n/a |

### 1.6 Integration after conquest

| Quantity or rule | Value or range | Source URL | Confidence |
| --- | --- | --- | --- |
| Gaul: conquest 58 to 50 BCE, great revolt 52 BCE; assimilation (Latin inscriptions, urbanism, elite citizenship) visible from the 1st century BCE and near-complete in signs by the end of the 1st century CE: **about 100 to 150 years** | ~100 to 150 years | https://britannica.com/place/France/The-Roman-conquest , https://www.thecollector.com/rome-vs-gaul/ | L |
| Dacia (106 CE): natives apparently not Romanized, through rejection or exclusion; violence of the wars and fast colonisation are both proposed causes | failure case | https://repository.arizona.edu/handle/10150/620700?show=full | M |
| HOI4: resistance target depends on development and on whether the core owner exists; compliance builds with time (more factories, resources) | rule | see 1.2 | M |
| CK3 county control: -1% tax and -0.5% levy per missing point of 100 | see 1.1 | see 1.1 | L |
| Effects on occupied pops (conscription, taxation, migration, assimilation rates) in numbers | **not found** | n/a | n/a |

### 1.7 Shrinking: secession, collapse, state birth and death

| Quantity or rule | Value or range | Source URL | Confidence |
| --- | --- | --- | --- |
| Europe had as many as about **500** independent states in 1500 and about **30** around 1900 (Tilly): net loss about 470 in 4 centuries, roughly **1 a year**; also quoted as ~25 | 500 to 30 (or 25) | https://en.wikipedia.org/wiki/Coercion,_Capital,_and_European_States,_AD_990%E2%80%931992 (search summary gave the numbers from a secondary page; check) | L |
| Fazal, State Death: half of the countries on an 1816 map no longer exist; state death has become rare since 1945 (norm against conquest); buffer states between rivals are the most likely to die | 50% in ~200 years | https://press.princeton.edu/node/61629 | M |
| Fazal and Griffiths: secession is a rising mode of state formation since WWII | rule | https://www.siwps.org/professor-tanisha-fazal-in-brown-journal-of-world-affairs/ | M |
| Civil war outbreak rate since 1945: about **2.3 a year** world-wide, no trend; civil wars end about 1.7 a year (Fearon and Laitin 2003) | 2.31 / yr | https://cpi.stanford.edu/_media/pdf/Reference%20Media/Fearon%20and%20Laitin_2003_Conflict,%20War,%20and%20Instability.pdf | M |
| Average empire lifetime about **215 years** in a dataset spanning 3,000 years (journalistic summary of Turchin) | 215 years | https://www.tampabay.com/archive/2011/11/13/by-the-numbers-doomed/ | L |
| Secular cycles last 100 to 300 years; more than 40 cycles detected in Eurasia and North Africa | 100 to 300 years | https://wiki.p2pfoundation.net/Secular_Cycles | L |
| Exact state deaths per century from Correlates of War | **not found** (only the 50% figure) | n/a | n/a |

### 1.8 Diplomacy

| Quantity or rule | Value or range | Source URL | Confidence |
| --- | --- | --- | --- |
| Walt: states ally to balance **threat** (power, proximity, offensive capability, perceived aggression), not just power; balancing is far more common than bandwagoning; bandwagoning becomes likely for a much weaker state, with no allies available, or once war has begun | rule | https://en.wikipedia.org/wiki/Balancing_(international_relations) , https://adambrown.info/p/notes/walt_the_origins_of_alliances | M |
| Games: Stellaris loyalty from negotiated terms and ethics; V3 liberty desire; CK3 opinion per obligation (-10 for high levy) and hooks; HOI4 factions (guildorder summary, not read) | see 1.3 | https://guildorder.com/it/games/hoi4/wiki/diplomacy-and-faction-mechanics | L |
| How often alliance obligations are honoured (ATOP) | **not found** (search returned unrelated pages) | n/a | n/a |

### 1.9 Slavery as a modelled fact (information only)

| Game | What is modelled | Source URL | Confidence |
| --- | --- | --- | --- |
| Victoria 3 | Enslaved people are pops with a distinct profession: no wages, no choice of work, bare-minimum goods; produces radicalisation, turmoil and uprisings. Developers defended including it for historical accuracy | https://www.pcgamer.com/victoria-3-devs-on-handling-slavery-in-a-historical-strategy-game/ , https://forum.paradoxplaza.com/forum/developer-diary/victoria-3-dev-diary-15-slavery.1490983/ | M |
| Stellaris | Several "servitude" and slavery-type options as labour and ethics systems; enslaved pops lack citizenship | https://phdns.latahcountyid.gov/article/stellaris-slavery-types-a-complete-guide (low-quality mirror; check on a Paradox page) | L |
| CK3 | **Not found** in the searches | n/a | n/a |

## 2. Equations and rules we could implement

All are **Proposals**, ours, built on the anchors above. Every parameter is a world-draw with a realistic band and a wider sandbox band, as in [sandbox parameters](../design/sandbox-parameters.md).

**R1. Control unit (cell).** A cell holds pops, buildings and resources (like a V3 state or CK3 county) and has `owner`, `controller`, `claimants[]`, `integration` in [0,1]. Faction totals are sums over cells it owns, weighted by `integration` and by `control` (CK3-style: yield x control).

**R2. Reach (loss-of-strength gradient).** Power projected to cell c from the nearest supplied base b:
`P(c) = P_total * f_supply * exp(-d(b,c) / L) * terrain(c)`.
`L` (the decay length) is a parameter: **no value found**; calibrate from logistics, e.g. L = days of march a supplied army sustains. Forward bases reset `d`. Realistic band: set L so a pre-industrial state projects half power at about 1 to 2 weeks of march; sandbox 0.2x to 5x. Boulding's own point (transport cuts the gradient) gives a rule: `L` grows with roads, rail and ships a faction has built (era as readout, not clock).

**R3. Battle outcome (aggregate).** Two forces with strength `S = N * q` (N heads from pops; q quality from equipment and training):
- Effective ratio `r = (S_att * m_att) / (S_def * m_def)` with `m` = terrain, fortification, supply, morale multipliers (HOI4 gives terrain examples: mountains -40% attack, large river -50%).
- Loss per day: `dN_a = -c_a * S_def`, `dN_d = -c_d * S_att` (Lanchester square, exponent 2), and blend toward the linear law by using `S^k` with `k` in [1,2], because history fits neither well (Dupuy critique). Band: k 1.0 to 2.0, default 1.5 (design, no source).
- A side **breaks** when morale falls below a threshold or supply is cut, before annihilation; retreating losses follow the loser. (Medieval sieges: most strongholds that fell did so within 2 weeks; use as a floor on siege duration for weak forts.)
- Noise: draw outcome from a logistic of `ln r` so r = 1 gives 50% and r = 3 gives a clear but not certain win (no source for the slope; calibrate on [war in other games](war-in-other-games.md) data).

**R4. Siege.** `T_siege = max(T_min, garrison_food / (garrison * ration) , (wall_strength) / (siege_power))`; the shorter of starvation and assault wins; a relieving army triggers R3 instead. Band: T_min about 1 day; most successful sieges under 14 days, long tail to months (West Point summary).

**R5. War score and exhaustion.** Following Stellaris: `score = share of enemy cells occupied (weighted by value) + battle wins`; `exhaustion` in [0,100]% rising with occupation of own cells, losses and time; at 100% a side accepts status quo or worse. No exhaustion constants found: draw bands (design).

**R6. Occupation and integration.** For a conquered cell: `resistance -> target` where target rises with unintegrated cultural distance, existence of a rival core owner (HOI4) and harsh law, and falls with garrison; `compliance` grows with time; yield `= base * compliance`; garrison cost drains manpower from the **occupier's** pops. Integration time: default about 1 to 3 generations for cultural assimilation (Gaul about 100 to 150 years, a low-confidence anchor), with a **failure case** if the occupier excludes locals (Dacia). Band: tax and levy fully available after 5 to 30 years (design), cultural convergence 100 to 150 years (Gaul).

**R7. Subject contract (one structure, many names).** A subject has: `tribute share t` of its revenue, `levy share v` of its manpower, `autonomy a` in [0,1] over foreign policy, `protection` given by the overlord. Presets: tributary (t small or negative, a high, as China: gifts exceeded tribute), vassal (t about 0.1 to 0.3, v 0.17 to 0.5, borrowed from CK3 16.6/25/33.3/50), puppet/protectorate (a low, t 0.3+, Stellaris minimum 30% for tributary-type). The numbers above are **game numbers**; the real historical figure found is one Ottoman sum, so treat realistic share bands as **not found**.

**R8. Subject loyalty and break-away (V3 liberty desire).** `libdesire = base + ethnic_distance + (t, v burdens) - protection_value - overlord_power_ratio_term + crisis`; `P(rebel per year) = sigmoid(libdesire - overlord_strength_term)`. Overlord weakness (civil war, defeat, lost supply) raises it, which produces the Assyrian pattern of defection when a rival rises. Absorption: when the overlord's cost of indirect rule exceeds the value of direct rule (or on a succession event, like Doctrine of Lapse), annex to a province (Assyria under Tiglath-pileser III).

**R9. Expansion by pressure.** Settlement frontier: a cell is settled when local `pop > K * threshold` and a neighbouring cell has free capacity; conquest is chosen when `value(target) / cost > theta`, with `cost` from R2 to R5 and `value` = target tax and manpower x integration; Carneiro rule: **bounded** land lowers the cost of subjecting the loser and raises the cost of leaving.

**R10. Secession and collapse.** Per cell and year, `P(secede) = sigmoid(unrest - control - garrison)`; cells with a rival culture and a claimant faction are first. Faction-level collapse when revenue cannot pay the armed forces for N seasons (debt) or legitimacy falls below a threshold. Calibration target (low confidence): civil wars start at about 2.3 a year among about 150 to 200 modern states (a rate near 1 to 1.5% a country-year; derived from Fearon and Laitin and the state count, check), empire mean life about 215 years, Europe lost about 1 state a year from 1500 to 1900.

**R11. Diplomacy.** Alliance value to X for ally Y: `threat_to_X(Y's enemy) = f(power, proximity, offensive capability, aggression)` (Walt); join the side that **balances** the strongest neighbouring threat; bandwagon only if much weaker with no allies available or war already running. Opinion as a sum of dated modifiers with decay (CK3, V3 style); deliberately no abstract diplomacy currency ([macro layer](../design/macro-layer.md) section 2).

## 3. How this closes the loop with pops and nations

- A cell's yield (tax, levy, food, officials) is computed from its pops and buildings; the faction never has a stat that is not a head count of someone ([world numbers](../design/world-numbers.md) section 1).
- Conscription and garrison duty remove workers from the occupier's own pops; war lowers output, which lowers supply, which lowers R2 reach: state -> pop -> state.
- Occupation drives resistance, flight and migration in the occupied pops; compliance and culture convergence feed back into their yields, so a conquest pays only after integration.
- A subject contract changes both sides' pops: tribute and levies leave the subject's economy; protection lowers its risk; liberty desire follows from the burden felt by its pops.
- Casualties and starvation change age and sex structure of cells ([demography note](world-numbers-demography.md)), which later limits recruits: a long war costs a generation.
- Faction size runs into its administrative reach (Stellaris empire size, CK3 control): unintegrated cells add cost before they add yield, so growth is limited by officials (literate pops), not by the map.
- Collapse of a faction returns cells to local control; the pops remain and form new factions around surviving offices, so history continues instead of resetting.
- The player-scale view (a survivor, a pop level of [spec 28](../tier0/specs/28-pop-levels.md)) reads the same cell state: security, tax pressure and conscription are what a person feels of the macro layer.

## 4. Unverified / not found

- Page-level verification of every game rule: WebFetch was blocked; all text is from search summaries. Patch-dependent numbers (HOI4 autonomy points, CK3 levy shares, Stellaris 30% minimum) may have changed.
- Not found: Victoria 3 tribute percentages and liberty-desire formula; CK3 war score and claim numbers; CK3 slavery treatment; HOI4 war score and resistance numeric targets; Stellaris empire size cap numbers.
- Not found: realistic tribute and revenue shares by empire (only Ming mission frequency and one Ottoman figure); subject rebellion frequencies and durations; Roman client-king annexation rate; number of British princely states (565 is commonly cited but did not appear in the summaries); exact dates of annexations other than Assyria.
- Not found: a decay constant for the loss-of-strength gradient; empirical Lanchester exponent for history; numeric siege-duration distribution (only "most under 2 weeks").
- Not found: state deaths per century from Correlates of War (only Fazal's half since 1816); Tilly's 500 to 25 or 30 figure is from a secondary summary (two values quoted); Seshat per-polity territory and duration series not opened; rates of occupied-pop conscription, taxation, migration and assimilation.
- The 215-year empire lifetime comes from a newspaper summary of Turchin; check against the primary book.
- Derived civil-war probability (R10) is our arithmetic and needs a proper state-year count.

## 5. Data licences

We use only public-domain or CC BY data. Nothing in this note is copied from a dataset; numbers are facts cited from publications, to be re-checked before use.

| Source | Licence | Usable? | URL |
| --- | --- | --- | --- |
| Seshat Global History Databank | CC BY-NC-SA (non-commercial) | **No** | https://seshat-db.com/downloads_page |
| Correlates of War | custom click-wrap: no commercial use, no redistribution without permission | **No** | https://correlatesofwar.org/?p=66 |
| HYDE (population and land use) | CC BY 4.0 as claimed in earlier notes; **not confirmed** by this search | Likely yes after checking | not found |
| Fearon and Laitin, Tilly, Turchin papers | publications: cite facts, do not copy datasets or tables | Facts only | URLs above |
| Paradox wikis and dev diaries | game documentation; use ideas and cite, do not copy text or data tables | Ideas only | URLs above |
| Wikipedia text | CC BY-SA (share-alike) | No copying; cite | URLs above |

Everything we ship must be generated from our own parameters with bands that cite the publication; no dataset file is imported.
