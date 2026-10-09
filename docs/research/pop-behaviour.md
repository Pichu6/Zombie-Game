# Pop behaviour from job, skills, traits and state: games and real life

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [0060](../decisions/0060-one-fight-three-views-behaviour-from-jobs-and-skills.md), [spec 24](../tier0/specs/24-pop-ai.md), [inhabitants](../design/inhabitants.md), [pop states and actions](../design/pop-states-actions.md). **Evidence:** read in full: Kevin Dill, "Dual-Utility Reasoning" (Game AI Pro 2, chapter 3, free PDF) and Jeff Orkin, "Three States and a Plan: The A.I. of F.E.A.R." (GDC 2006 paper). Everything else is from web-search summaries of wikis, papers, abstracts and press (`reported`): the GDC Vault talks by Dave Mark and Kevin Dill are members-only, the Paradox wiki and the CK3 game files were not read, and most disaster papers were seen as abstracts only. Numbers to re-check before they go to `data/`: PB-22, PB-31, PB-35, PB-37, PB-38, PB-40. **IDs:** PB.

Decision 0060 says a pop acts from who it is: job, skills, profession, traits and state, the same with or without a player above it. Spec 24 has a small utility AI with two fixed traits and no skills. This note collects how games build such pops and what real people do, especially in disasters, and ends with a proposal for the pop behaviour model and its tests.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| PB-1 | Utility AI scores every option at decision time with a heuristic and picks from the scores, instead of the yes-or-no questions of a state machine or behaviour tree | sourced | Dill, Game AI Pro 2 ch. 3 |
| PB-2 | Dave Mark and Kevin Dill's GDC 2010 AI Summit talk proposed response curves (a curve turns an input such as distance or hunger into a 0-1 score), population distributions and weighted randoms to widen the decision space and handle edge cases | reported | gdcvault.com/play/1012410; en.wikipedia.org/wiki/Utility_system |
| PB-3 | Pure weighted random looks stupid because low-weight options still get picked sometimes; squaring weights or cutting the lowest only half fixes it | sourced | Dill, Game AI Pro 2 ch. 3 |
| PB-4 | Dual utility gives each option a rank and a weight: drop zero-weight options, keep only the highest rank present, drop options much weaker than the best, then weighted random among the rest | sourced | Dill, Game AI Pro 2 ch. 3 |
| PB-5 | In Zoo Tycoon 2 everyday need options had rank 0 and varied weight; a situation with its own behaviours (a koala in a tree) raised those to rank about 5 so the animal did not drop out of the tree; scripted shows used ranks near 100 | sourced | Dill, Game AI Pro 2 ch. 3 |
| PB-6 | F.E.A.R.'s character state machine has three states (Goto, Animate, UseSmartObject); the logic of when to switch lives in a planner that chains actions with A* to satisfy goals | sourced | Orkin 2006 |
| PB-7 | In F.E.A.R. a soldier, an assassin and a rat given the same goals (Patrol, KillEnemy) behave differently because each has a different action set; the rat has no attack action, finds no plan for KillEnemy and falls back to Patrol | sourced | Orkin 2006 |
| PB-8 | Orkin names decoupling goals from actions as the first benefit of planning: types of character satisfy the same goal in their own way without new decision code | sourced | Orkin 2006 |
| PB-9 | F.E.A.R. makes coordination visible by having AI speak their intent; "I need reinforcements" is said with no reinforcement system behind it, and players read later enemies as the reinforcements | sourced | Orkin 2006 |
| PB-10 | Halo 2 popularised behaviour trees: a prioritised list of behaviours (search, combat, flight, self-preservation, idle) about four levels deep, with "impulses" for extra triggers | reported | Isla, GDC 2005, via en.wikipedia.org Behavior tree and WPI lecture slides |
| PB-11 | The Sims' needs and object advertisements are already in [SIMS-1](the-sims.md), [SIMS-2](the-sims.md), [SIMS-5](the-sims.md) (unequal weights and some randomness) and [SIMS-9](the-sims.md); not repeated here | reported | [the-sims.md](the-sims.md) |
| PB-12 | RimWorld passion multiplies skill XP: none 35 %, interested 100 % (+8 mood while doing it), burning 150 % (+14 mood); a soft cap of 4,000 XP per skill per day, gains above it at 20 % | reported | rimworldwiki.com/wiki/Skills, Passion |
| PB-13 | RimWorld skills above level 10 lose XP all the time, faster at higher levels; a level is lost after XP falls 1,000 below zero; XP comes from time spent, not work done | reported | rimworldwiki.com/wiki/Skills |
| PB-14 | RimWorld's mental break threshold is 35 % mood by default; major is 4/7 and extreme 1/7 of it; Iron-willed -18 %, Steadfast -9 %, Nervous +8 %, Volatile +15 % (one per pawn) | reported | rimworldwiki.com/wiki/Mental_Break_Threshold |
| PB-15 | RimWorld breaks come in three tiers with their own pools: minor (hide in the room for a day), major, extreme (up to murder); a break makes the pawn uncontrollable for a time | reported | rimworldwiki.com/wiki/Mental_break |
| PB-16 | RimWorld traits are signed stat offsets: work speed Industrious +35 %, Hard worker +20 %, Lazy -20 %, Slothful -35 %, Neurotic +20 % (with more break risk), Very neurotic +40 %; Tough halves damage taken; some traits also change opinions of others | reported | rimworldwiki.com/wiki/Global_Work_Speed; trait tier lists |
| PB-17 | RimWorld backstories can make a pawn incapable of whole work types (for example violence or caring); the player sees this on the pawn card | reported | general knowledge of RimWorld, rimworldwiki.com |
| PB-18 | Dwarf Fortress personality facets run 0-100; only values outside 40-60 are described in the game; beliefs (values) are separate from facets; both set which needs a dwarf has and how often they must be met | reported | dwarffortresswiki.org Personality, Facet, DF2014:Need |
| PB-19 | Dwarf Fortress stress has a short-term and a long-term part (long-term -50,000 to 120,000); breakdowns need both high; bravery sets how fast stress builds, stress vulnerability how much it takes to break, anxiety how fast it drains; memories can shift facets over time | reported | dwarffortresswiki.org/Stress |
| PB-20 | Dwarf Fortress skills: Novice at 500 XP, each next level 100 XP more, Legendary at level 15 and up; unused skills rust after about 4.5 months, rust peaks at about 2 years 3 months, real levels decay after 6 years; any practice removes rust | reported | dwarffortresswiki.org/Experience, Rust (older versions) |
| PB-21 | Crusader Kings III AI personality uses values such as boldness, rationality, greed and energy, set by traits; boldness sets how dread affects the character and how soon it attempts a scheme | reported | segmentnext.com CK3 personalities; Steam guides |
| PB-22 | CK3 stress is a gauge of mental well-being; stress levels 1-3 add growing health and fertility penalties and can lead to breaks; acting against one's traits adds stress, and coping mechanisms relieve it | reported | Paradox forum, CK3 Dev Diary 31 "A Stressful Situation" |
| PB-23 | Football Manager rates players 1-20 on visible and hidden attributes; hidden personality attributes (ambition, loyalty, pressure, professionalism, sportsmanship, temperament, controversy) plus consistency and important matches; scouts report them as words ("enjoys big matches"), never numbers | reported | fmscout.com personality guide; sortitoutsi.net hidden attributes |
| PB-24 | Football Manager players believe professionalism and determination mostly drive development, consistency and important matches drive match-day variance; none of it is officially documented | reported | same as PB-23 |
| PB-25 | Kenshi trains each stat by doing it; toughness rises from being hurt and beaten and from getting back up in a fight; a higher toughness speeds the growth of other combat skills | reported | kenshi.fandom.com/wiki/Toughness; Steam discussions app 233860 |
| PB-26 | Project Zomboid occupations and traits are bought with points at creation and skills grow by use; see [PZ-5](project-zomboid.md) and [PZ-16](project-zomboid.md); not repeated here | reported | [project-zomboid.md](project-zomboid.md) |
| PB-27 | Oxygen Not Included: each duplicant has one fixed stress reaction (ugly crier, vomiter, destructive, binge eater) that fires at 100 % stress, with milder task dropping near 50 %; expectations rise as skills are learned, so a skilled dupe needs better food and rooms | reported | Klei forums; gamepressure.com ONI duplicant guide |
| PB-28 | Prison Architect reform programs (education, apprenticeships, behavioural therapy) run on a schedule; badly treated prisoners volunteer less; passing behavioural therapy lowers later trouble; a DLC lets programs remove negative traits | reported | prison-architect.fandom.com Alpha 18; Second Chances store page |
| PB-29 | Games show traits as named chips whose tooltip lists signed effects (RimWorld, CK3, Project Zomboid point costs, ONI's good and bad traits), and hide the numbers behind words when the game wants doubt (Football Manager scout reports, Dwarf Fortress facet sentences) | reported | PB-16, PB-18, PB-23, [PZ-5](project-zomboid.md) |
| PB-30 | Power law of practice: log time per trial falls linearly with log trials (Newell and Rosenbloom 1981, first proposed by Snoddy 1928) | reported | en.wikipedia.org/wiki/Power_law_of_practice |
| PB-31 | Heathcote, Brown and Mewhort (2000) fit 7,910 learning series from 475 people: an exponential curve fit every unaveraged set better; averaging people makes the curve look like a power law | reported | Psychonomic Bulletin and Review 2000, abstract via newcastle.edu.au |
| PB-32 | Deliberate practice explains 26 % of performance variance in games, 21 % in music, 18 % in sports, 4 % in education and under 1 % in professions (Macnamara, Hambrick, Oswald 2014, 157 effect sizes) | reported | Psychological Science 2014; PMC4101876 |
| PB-33 | A US registered apprenticeship is usually 4 years, about 2,000 hours of on-the-job work and 144-160 classroom hours a year (electrician, plumber, carpenter); Ontario's electrician is 9,000 hours | reported | bls.gov Career Outlook 2013; skilledtradesontario.ca; college program pages |
| PB-34 | Conscientiousness is the one Big Five trait that predicts job performance in every occupational group (Barrick and Mount 1991, 117 studies; about r = 0.2); extraversion helps only in social jobs such as sales and management; the other traits mostly under 0.1 | reported | Personnel Psychology 1991, abstract; secondary summaries |
| PB-35 | Personality predicts mortality, divorce and job attainment about as well as socioeconomic status and cognitive ability do (Roberts et al. 2007); a replication of 78 trait-outcome links held about 85 % of them at about 80 % of the original size | reported | Perspectives on Psychological Science 2007; Soto 2019 (LOOPR) |
| PB-36 | Quarantelli lists the disaster myths that research rejects: mass panic, helplessness, social breakdown, role abandonment by officials, and looting | reported | Quarantelli 2008, "Conventional Beliefs and Counterintuitive Realities", udspace.udel.edu |
| PB-37 | Fritz (1961) argued disasters often produce socially healthy conditions: a "therapeutic community" of shared danger and mutual help; later authors (Barton 1970) developed it | reported | Fritz 1961, republished 1996, udspace.udel.edu; en.wikipedia.org/wiki/Charles_Fritz |
| PB-38 | In 141 survivor accounts of the 2005 London bombings, 140 reported seeing help and mutual aid and 3 saw selfish acts; a shared sense of "us" went with helping and with risk taken for strangers (Drury, Cocking, Reicher) | reported | secondary summary; cris.brighton.ac.uk draft; sussex.ac.uk |
| PB-39 | First responders believe some of the myths (panic, mass flight, disorder) more than the public does (Alkalai Tavori and Adini 2024) | reported | med.tau.ac.il PDF of the 2024 paper |
| PB-40 | Mawson's social attachment model: under threat people mostly move toward familiar people and places, not away from danger; separation from attachment figures stresses more than physical danger; flight is likelier when they are absent | reported | Mawson 2005, Psychiatry, csap.cam.ac.uk PDF |
| PB-41 | A post-fire study found people with close others delayed evacuation and did more affiliating; helping rose with perceived danger whether close others were there or not | reported | sbp-journal.com article 6946 |
| PB-42 | Killian's early role-conflict paper (about 1952) assumed workers put family before job in a disaster; Dynes and Quarantelli (1977) found little behavioural evidence of it in six disasters; White (1962) titled hers "Not family but familiarity first"; Trainor and Barsky (2011) reviewed over 100 reports | reported | udspace.udel.edu handles 19716/483, 19716/9885 |
| PB-43 | Hurricane Katrina: New Orleans police put the officers who left their posts at 249, about 15 % of the force; 51 staff were fired for desertion; a federal report spoke of about a tenth not reporting | reported | nbcnews.com wbna9855340; police1.com |
| PB-44 | Balicer et al. (2006, Maryland, 308 public health staff): over 40 % said they were unlikely to report in a flu pandemic; those who saw their role as important (mostly clinicians) were 86.8 % willing; support staff least willing | reported | BMC Public Health 2006 via eurekalert.org |
| PB-45 | A 2015 systematic review found healthcare workers' stated willingness to work in a pandemic ranges 23.1 % to 95.8 % across studies, too varied to pool (I² 99.2 %); fear for self and family is the main barrier | reported | Aoyagi et al. 2015, PMC4415696 |
| PB-46 | Actual absence: in Hong Kong hospitals the 2009 pandemic raised staff sickness absence 57.7 % over baseline (seasonal flu +8.4 %); in an Australian survey 56.6 % of emergency staff missed at least one day | reported | Disaster Medicine and Public Health Preparedness; istex record, search summaries |
| PB-47 | S.L.A. Marshall (1947) claimed no more than a quarter of infantry fired in combat, 55 % in Korea; Spiller (1988) found no records of the systematic interviews; Engen found most Canadian infantry did fire | reported | Spiller 1988 RUSI Journal (gwern.net); Engen 2011 Canadian Military History |
| PB-48 | Grossman's "innate resistance to killing" rests largely on Marshall; Engen argues it has little support in biology or psychology, though he credits Grossman's notes on the physiology of combat | reported | Engen 2008, Canadian Military Journal (gwern.net) |
| PB-49 | Tierney, Bevc and Kuligowski (2006) argue media greatly exaggerated looting after Katrina; looting in US natural disasters is rare and often takes necessities; Frailing (2007) argues looting is real and measurable after some storms | reported | Annals AAPSS 2006 (ideas.repec.org); Natural Hazards Observer March 2007 |
| PB-50 | Milgram's 65 % full obedience was one condition; across his conditions the average was about 43 %; Burger's 2009 partial replication had 70 % go past 150 V against Milgram's 82.5 %; Perry found the experimenter often went off script | reported | forrt.org obedience chapter; Burger 2009 via scu.edu; Perry 2013 |
| PB-51 | Evacuation compliance varies widely: about 70 % of Miami-Dade barrier island residents left before Andrew (1992), 47 % under a mandatory order for Floyd (1999); Sandy: 42.5 % under mandatory orders vs 11.8 % without; one review gives 12-97 % across hurricanes | reported | newswise.com (Baker); Cambridge DMPHP Sandy survey; MDPI review |
| PB-52 | The Protective Action Decision Model: people first receive, notice and understand a warning, then judge the threat, the protective action and the source; situational helps and hindrances turn that into behaviour; a large share in every group waits to see | reported | Lindell and Perry 2012, Risk Analysis (training.weather.gov PDF); wildfire study pure.jgu.edu.in |
| PB-53 | NIST's World Trade Center survivor study modelled evacuation start delay as its own outcome: people did not leave at once but gathered information, collected things and talked first | reported | NIST NCSTAR 1-7; Kuligowski, Peacock, Averill 2013 |
| PB-54 | Games converge on: a duty or situation layer that wins outright (rank, goal priority, prioritised tree), needs and traits that weigh options inside it, and a few loud failure states (breaks) shown with a reason | derived | PB-4, PB-5, PB-7, PB-10, PB-14, PB-27 |
| PB-55 | Real crises show the opposite of game breaks: panic and selfishness are rare, helping is the norm, and the big real failures are delay (waiting, milling, seeking family) and not showing up for duty when family or contagion is at stake (10-15 % police, 20-60 % health staff) | derived | PB-36, PB-38, PB-40, PB-43, PB-45, PB-46, PB-52, PB-53 |
| PB-56 | Learning is fast at first and slows; per person it is closer to exponential; practice explains only part of skill, so pops should differ in learning rate and ceiling | derived | PB-30, PB-31, PB-32 |
| PB-57 | Traits move behaviour modestly (r about 0.2 for the strongest work trait): a pop's job, skill and situation should explain most of what it does, traits the rest | derived | PB-34, PB-35 |

## What this means for us (Proposal)

All rows below are `estimate`, tied to the findings by ID. They are a proposal for spec 24 and the inhabitants doc, not a decision.

### 1. The pop record

| Part | What it holds | Why |
| --- | --- | --- |
| **Dispositions** (hidden) | 6 numbers 0-1: conscientiousness, sociability, agreeableness, anxiety (stress gain), resilience (stress limit), boldness (fear gain). Drawn per pop at generation, correlated with age | Big Five plus the three DF stress traits ([PB-19](#findings), [PB-34](#findings)) |
| **Traits** (shown) | A word shown only when a disposition is outside the middle band (DF's 40-60 rule, [PB-18](#findings)), plus a few discrete traits from life (smoker, honest/secretive, bad back). Each trait lists its signed effects | [PB-16](#findings), [PB-29](#findings) |
| **Skills** | Per procedure (spec 24 knowledge, inhabitants "learning"): practice hours, a personal rate and ceiling, rust timer | [PB-13](#findings), [PB-20](#findings), [PB-56](#findings) |
| **Occupation** | Census job: a duty set (what it must do in a crisis and in daily work), an action set (what it knows how to do), a workplace, a perceived role importance | [PB-7](#findings), [PB-44](#findings) |
| **Attachments** | Household and bonds (inhabitants) with "known whereabouts" per person | [PB-40](#findings), [PB-42](#findings) |
| **State** | Body needs (spec 11), fear and morale (spec 11, 24), short and long stress | [PB-19](#findings) |

Rules for the effects (estimate):

- **Signed and small.** Every trait effect is a multiplier or offset on a named quantity: work speed, fear gain, stress limit, aim spread, learning rate, opinion. Sizes stay in the RimWorld band of 10-35 % ([PB-16](#findings)) and never decide alone ([PB-57](#findings)).
- **Every trait has a plus and a minus** where reality allows: anxious pops notice threats sooner but stress faster; bold pops hold under fire but step into danger; conscientious pops work steadier but stress when duty and family clash. This gives the Individual view honest pluses and minuses ([0060](../decisions/0060-one-fight-three-views-behaviour-from-jobs-and-skills.md) item 4).
- **Incapacities are rare and real** (a pop that cannot treat a wound because it faints at blood), not RimWorld's whole-work-type bans ([PB-17](#findings)).

### 2. Skills learned by doing, on a real curve

- **Curve:** quality from practice hours h follows q = c (1 - exp(-h / tau)), per pop and per skill ([PB-31](#findings)). c (ceiling) and tau (time constant) vary by pop around a skill mean, so practice explains only part of the spread ([PB-32](#findings)).
- **Time constants from trades:** a trade reaches journeyman (q about 0.8 c) at about 8,000 practice hours, so tau about 5,000 h; first aid basics in about 10-20 h; weapon handling basics in tens of hours ([PB-33](#findings); the short ones are estimates to source later).
- **Census start:** a pop's job and years worked give its starting hours, so a 20-year electrician starts near its ceiling and a nurse knows wound care ([PB-33](#findings)).
- **Learning rate:** novelty pays, repetition pays less (inhabitants, Decided); conscientiousness and interest (a RimWorld-like passion, [PB-12](#findings)) multiply it by 0.5-1.5; a daily cap stops grinding.
- **Rust:** unused skills lose speed after months and level after years (DF, [PB-20](#findings)); knowledge of a procedure is never lost, only fluency.
- Skill acts on quality and speed of a process (Haven and Hearth soft cap, inhabitants), and on fear: a trained pop gains less fear doing its trained task (the main real effect behind soldiers and nurses holding, [PB-44](#findings), [PB-47](#findings)).

### 3. Occupation, duty and role conflict

- **Job = action set, goals shared** (F.E.A.R., [PB-7](#findings), [PB-8](#findings)): every pop has the same goals (stay alive, keep household safe, do duty, meet needs); the job adds actions (soldier: suppress, hold a line; nurse: triage, treat; farmer: none for fighting beyond a tool as a weapon). A farmer with the goal "defend" and no fighting actions falls back to flee or hide, like the rat.
- **Duty is a rank, not a weight** (dual utility, [PB-4](#findings), [PB-5](#findings)): when the pop is on duty in a crisis, duty options sit one rank above daily needs and below survival reflexes. Inside a rank, needs, traits and skill set the weights.
- **Role conflict as a test of the rank:** a duty-bound pop whose household is in danger and unaccounted for may drop duty to find them. Probability rises with attachment and contagion fear, falls with perceived role importance and with knowing the family is safe ([PB-40](#findings), [PB-42](#findings), [PB-44](#findings), [PB-45](#findings)). Target bands: about 10-15 % of police in a flood-scale disaster ([PB-43](#findings)); 20-50 % of health staff absent at the peak of a contagious outbreak ([PB-45](#findings), [PB-46](#findings)); much less for soldiers in units with their own supply.
- **Familiarity first** ([PB-42](#findings)): a pop answers the crisis nearest to what it knows: a nurse treats the wounded next to her even off shift.

### 4. Needs, fear and stress

- Needs stay as body readouts (spec 11, [SIMS-10](the-sims.md)).
- **Fear** is fast (seconds to minutes, spec 24 already); **stress** is slow: short-term stress from events, long-term from accumulated short-term, drained by sleep, company and coping ([PB-19](#findings), [PB-22](#findings)).
- **Breaks are rare and quiet.** Real disasters show little panic ([PB-36](#findings), [PB-38](#findings)), so the common "break" is withdrawal, freezing, refusing an order or leaving to find family, not berserk rages. Loud breaks (violence, looting beyond need) need both high long-term stress and specific traits and stay rare ([PB-49](#findings)).
- **Acting against one's traits or duty adds stress** (CK3, [PB-22](#findings)): a conscientious pop that abandons its post, or an agreeable one ordered to shoot a bitten friend.

### 5. How the AI chooses

A per-pop dual-utility step every 5 s (the current spec 24 cadence):

| Rank | Options | Notes |
| --- | --- | --- |
| 5 | Survival reflex: flee, fight back, take cover | From fear and threat in reach (spec 24 fight or flee) |
| 4 | Urgent body: bleeding, cannot breathe | Spec 24 want 1 |
| 3 | Duty in a crisis (on shift or called) and squad orders | Rank drops to 2 if the role-conflict test fails |
| 2 | Household and attachments: find, protect, gather them | [PB-40](#findings) |
| 1 | Needs, daily job, help nearby strangers | Helping weighted up by danger and shared identity ([PB-38](#findings), [PB-41](#findings)) |
| 0 | Idle, social, wait and see | Waiting and milling are real choices ([PB-52](#findings), [PB-53](#findings)) |

- Weights are response curves of needs, distance, skill (a skilled pop scores its trained action higher), traits and stress ([PB-2](#findings)).
- Drop options much weaker than the best before the random pick ([PB-3](#findings), [PB-4](#findings)), so pops are varied but never stupid.
- **Obedience:** an order is an option at the order rank whose weight depends on authority, trust and harm; a harmful order is obeyed by a minority to a slim majority, not by all ([PB-50](#findings)). Evacuation orders raise leaving but do not force it ([PB-51](#findings)).
- **Speak intent** in the Individual and City views ("going for my kids", "holding the door"), so behaviour reads as chosen ([PB-9](#findings)).
- The same step runs for possessed pops' companions, officeholders and pops nobody watches (0058, 0060 item 3); at lower detail it becomes rates calibrated from the same tests.

### 6. What each layer shows (0060 item 4)

| Layer | Shows | Notes |
| --- | --- | --- |
| Individual | Trait chips with + and - effect lines on hover; skills as words (novice to master) with hours and rust; occupation and duties; needs (moodle column, [PZ-13](project-zomboid.md)); fear and stress as words; the current rank and reason ("on duty: holding the gate") | Numbers on hover only; dispositions in the middle band show nothing ([PB-18](#findings), [PB-29](#findings)) |
| City | Card: name, job, mood word, health word, doing, and one flag when relevant: "off post", "looking for family", "broke" | One flag, no traits ([PB-23](#findings) style: words not numbers) |
| Macro | Counts by behaviour class: on duty, absent from duty, evacuating, sheltering, helping, broke, dead | The calibration targets below become visible here |

### 7. Tests (A3 style, many seeds)

| Test | Pass band | Findings |
| --- | --- | --- |
| Crisis mix: a city of generated pops meets a sudden outbreak | Panic flight and violent breaks under 5 % of pops in the first day; helping others is the most common non-self action; most pops delay leaving to gather family or information | [PB-36](#findings), [PB-38](#findings), [PB-40](#findings), [PB-53](#findings) |
| Duty abandonment | Police-like jobs 5-20 % off post in the first week; health staff absent 20-50 % at the peak of a contagious phase; higher where household is at risk | [PB-43](#findings), [PB-45](#findings), [PB-46](#findings) |
| Evacuation order | 40-75 % of those ordered leave within the window; under 20 % without an order | [PB-51](#findings) |
| Soldiers under fire | A clear majority of trained soldiers fire when they have targets; not Marshall's 25 % | [PB-47](#findings) |
| Learning | Time to reach 0.8 c within ±25 % of the trade figure; per-pop curves exponential | [PB-31](#findings), [PB-33](#findings) |
| Trait size | Across a population, conscientiousness vs job output correlation 0.1-0.3, not higher | [PB-34](#findings), [PB-57](#findings) |
| Same pop, three resolutions | Behaviour class shares from the per-pop AI and from the City and Macro rates agree on average | 0060 item 2 |
| Possession | Releasing a pop keeps traits, skills, stress, duty state (spec 24 rule 8) | spec 24 |

## Open questions

1. How many hidden dispositions: six numbers, full Big Five plus stress traits, or DF-style dozens of facets? Fewer is cheaper to save per pop (pop storage notes).
2. Should a player see a stranger's traits at once, or learn them by observing (FM scouting, [PB-23](#findings))?
3. Does the player's own character get traits by the same draw, or chosen at creation with points (PZ)?
4. Role conflict needs to know where family is: does the AI use the pop's belief (what it knows) or the truth? Spec 24 has no belief store in Tier 0.
5. Does stress change dispositions over months (DF memories, [PB-19](#findings))? It would make long play deeper but break the calibration over time.
6. Looting and crime: how much comes from need (rare, [PB-49](#findings)) and how much from pre-existing criminals in the census? Needs a crime source.
7. Soldier units: do they get unit cohesion as its own number, or is it bonds plus training?
8. Short-skill time constants (first aid, firearms, driving) are guesses; they need sources before going to `data/`.

## Sources

- Dill, K. "Dual-Utility Reasoning", Game AI Pro 2, ch. 3: gameaipro.com/GameAIPro2/GameAIPro2_Chapter03_Dual-Utility_Reasoning.pdf
- Orkin, J. "Three States and a Plan: The A.I. of F.E.A.R.", GDC 2006: pages.cs.wisc.edu/~dyer/cs540/handouts/gdc2006_orkin_jeff_fear.pdf
- Mark, D. and Dill, K. "Improving AI Decision Modeling Through Utility Theory", GDC 2010: gdcvault.com/play/1012410
- Utility system: en.wikipedia.org/wiki/Utility_system
- Isla, D. "Handling Complexity in the Halo 2 AI", GDC 2005, via en.wikipedia.org/wiki/Behavior_tree_(artificial_intelligence,_robotics_and_control) and web.cs.wpi.edu/~rich/courses/imgd4000-d09/lectures/halo3.pdf
- RimWorld wiki: rimworldwiki.com/wiki/Skills, /Passion, /Mental_break, /Mental_Break_Threshold, /Global_Work_Speed
- Dwarf Fortress wiki: dwarffortresswiki.org/Personality, /index.php/Facet, /index.php/DF2014:Need, /Stress, /Experience, /index.php/Rust
- CK3: segmentnext.com/crusader-kings-3-characters-and-personalities; Paradox forum CK3 Dev Diary 31 "A Stressful Situation"
- Football Manager: fmscout.com/a-guide-to-player-personalities-football-manager.html; sortitoutsi.net/content/74854/fm26-hidden-attributes-explained
- Kenshi: kenshi.fandom.com/wiki/Toughness
- Oxygen Not Included: gamepressure.com/oxygen-not-included/profile/z59d44; forums.kleientertainment.com/forums/topic/103546
- Prison Architect: prison-architect.fandom.com/wiki/Alpha_18
- Power law of practice: en.wikipedia.org/wiki/Power_law_of_practice; Heathcote, Brown, Mewhort 2000, Psychonomic Bulletin and Review
- Macnamara, Hambrick, Oswald 2014: pmc.ncbi.nlm.nih.gov/articles/PMC4101876 (and Psychological Science 25(8))
- Apprenticeships: bls.gov/careeroutlook/2013/summer/art01.pdf; skilledtradesontario.ca
- Barrick and Mount 1991, Personnel Psychology 44; Roberts et al. 2007, Perspectives on Psychological Science; Soto 2019, Psychological Science (LOOPR)
- Quarantelli 2008: udspace.udel.edu/items/884efb25-3141-4c48-af83-6845586145c2
- Fritz 1961/1996: udspace.udel.edu/handle/19716/1325; en.wikipedia.org/wiki/Charles_Fritz
- Drury, Cocking, Reicher on 7/7: cris.brighton.ac.uk/ws/portalfiles/portal/305764; blogs.sussex.ac.uk/crowdsidentities/7/
- Alkalai Tavori and Adini 2024: med.tau.ac.il PDF (International Journal of Disaster Risk Reduction)
- Mawson 2005: csap.cam.ac.uk/media/uploads/files/1/mawson-2005-emotional-attachments.pdf
- Post-fire attachment study: sbp-journal.com/index.php/sbp/article/view/6946
- Dynes and Quarantelli 1977, role conflict: udspace.udel.edu/handle/19716/483; White 1962 and Trainor and Barsky 2011: udspace.udel.edu/handle/19716/9885
- NOPD after Katrina: nbcnews.com/id/wbna9855340; police1.com "Chief: 249 New Orleans officers left posts"
- Balicer et al. 2006: eurekalert.org/news-releases/738705; BMC Public Health
- Aoyagi et al. 2015: pmc.ncbi.nlm.nih.gov/articles/PMC4415696
- H1N1 absence: Disaster Medicine and Public Health Preparedness (Hong Kong records); Australian emergency staff survey (istex record)
- Marshall, Spiller, Engen: gwern.net/doc/history/s-l-a-marshall/1988-spiller.pdf; scholars.wlu.ca/cmh/vol20/iss4/4; gwern.net/doc/history/s-l-a-marshall/2008-engen.pdf
- Tierney, Bevc, Kuligowski 2006: ideas.repec.org/a/sae/anname/v604y2006i1p57-81.html; Natural Hazards Observer March 2007: hazards.colorado.edu/uploads/observer/2007/mar07/mar07.pdf
- Milgram, Burger, Perry: forrt.org/open-social-psychology/chapter4.html; scu.edu Burger PDF
- Evacuation compliance: newswise.com "Andrew had little effect on hurricane evacuation response"; cambridge.org DMPHP Hurricane Sandy survey; mdpi.com/2076-3417/13/8/5194
- Lindell and Perry 2012, PADM: training.weather.gov PDF; wildfire study pure.jgu.edu.in/id/eprint/355
- NIST NCSTAR 1-7: nvlpubs.nist.gov/nistpubs/Legacy/NCSTAR/ncstar1-7.pdf
- Our notes: [the-sims.md](the-sims.md), [project-zomboid.md](project-zomboid.md)
