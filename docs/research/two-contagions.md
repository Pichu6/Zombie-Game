# Two contagions: the outbreak and what people know about it

**Date:** 2026-10-10. **Status:** research note, not a decision. **Feeds:** [spec 38](../tier0/specs/38-outbreak.md) (the encounter reads `warning`), [spec 39](../tier0/specs/39-orders-and-belief.md) (beliefs, broadcasts, comms), [spec 24](../tier0/specs/24-pop-ai.md), [world simulation](../design/world-simulation.md) (§5, the outbreak per region), [threats](../design/threats.md), [layer jumps](../design/layer-jumps.md) (rows 14–16: outbreak, orders, news and belief), research [pop-behaviour](pop-behaviour.md) (PB: little panic, milling, obedience), [coarse-layers-from-fine-rules](coarse-layers-from-fine-rules.md) (EF: this is its first test world). **Evidence:**
- **Papers read in the body:** Granell, Gómez and Arenas 2013 (arXiv text) and Epstein et al. 2008 (PLoS ONE). Wang et al. 2015 and Siettos 2010 were read in part.
- **Abstracts or summaries only (`reported`):** Funk 2009, Funk 2010, Fenichel 2011, Hébert-Dufresne 2015, Mileti and Sorensen 1990, PADM, milling, Daley–Kendall, Centola, Vosoughi, Roozenbeek and the WHO framework. PMC answered with a captcha and several are paywalled.
- **Talks:** the transcripts of 15 talks (TC-35 to TC-64) (YouTube captions, kept outside the repo) were read in full; findings from talks cite the video and the minute.
- **IDs:** TC.

The outbreak spreads two things: the disease, and what people know about it. Each shapes the other. Spec 38 already shows the coupling in one number: a slow zombie among warned people fizzles, a fast one among unwarned people takes off. Today `warning` is fixed per run. This note asks what is known about the information contagion and how the two couple, so that **information becomes the player's main lever** (warnings, rumours, cover-ups) at every level of detail.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| TC-1 | UAU-SIS on a two-layer multiplex: awareness (unaware, aware) spreads on a communication layer and SIS on a contact layer over the same people; an infected person becomes aware at once, so only three states exist (unaware-susceptible, aware-susceptible, aware-infected) | sourced | Granell, Gómez, Arenas 2013, *PRL* 111:128701, https://arxiv.org/abs/1306.4136 |
| TC-2 | Aware people are infected at γ × the unaware rate (the figures use γ = 0, full protection); awareness passes along a link with probability λ per step and is forgotten with probability δ | sourced | Granell et al. 2013 |
| TC-3 | Its microscopic Markov chain gives each person the chance that no neighbour informs them and the chances that no neighbour infects them if aware or unaware, with no dynamical correlations assumed | sourced | Granell et al. 2013 |
| TC-4 | The epidemic threshold depends on every person's stationary awareness: β_c = μ / Λ_max(H), where H is the contact matrix weighted by each person's chance of being unaware | sourced | Granell et al. 2013 |
| TC-5 | A meta-critical point: while awareness is below its own threshold (λ < δ / Λ_max of the communication layer), the epidemic onset ignores awareness; above it, the onset rises and incidence falls. The effect disappears if either process cannot be undone (no forgetting, or no recovery) | sourced | Granell et al. 2013 |
| TC-6 | For some transmission rates the number infected first grows, then rising awareness drives the infection back towards extinction | sourced | Granell et al. 2013 |
| TC-7 | The Markov chain matches Monte Carlo within 2.5 % on threshold and incidence (1.6 % over the whole 100 × 100 phase diagram), on 1,000-node networks with 50 runs per point | sourced | Granell et al. 2013 |
| TC-8 | A fear-and-disease model with seven states: disease comes only from the sick, fear from the sick and the frightened, so fear has more sources; the frightened withdraw, and return when fear fades | sourced | Epstein, Parker, Cummings, Hammond 2008, *PLoS ONE* 3:e3955, https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0003955 |
| TC-9 | With equal transmission rates, fear rises before the disease because more states spread it | sourced | Epstein et al. 2008 |
| TC-10 | Withdrawing while infection rises and returning as it falls produces a second wave | sourced | Epstein et al. 2008 |
| TC-11 | In its agent model (1,800 agents, 30 runs each): all ignorers 99.9 % incidence; all hiders 27.8 %; 90 % hiders with 10 % ignorers 32 %; 90 % hiders with 10 % fleers 64 % | sourced | Epstein et al. 2008 |
| TC-12 | Fleers carry the pathogen past the buffer of frightened hiders: the epidemic reached the far corner in 92 % of runs with 10 % fleers against 17 % with all hiders, in about 340 periods instead of 600 | sourced | Epstein et al. 2008 |
| TC-13 | The behaviour–disease loop: prevalence raises perceived risk, which raises protection, which lowers transmission; models differ in whether the information is local, from contacts, or global | sourced | Wang, Andrews, Wu, Wang, Bauch 2015, *Phys. Life Rev.* 15:1–29, https://pmc.ncbi.nlm.nih.gov/articles/PMC7105224/ (partial read) |
| TC-14 | Global awareness does not lower the chance that an outbreak occurs; local and contact awareness do | reported | Wu et al. 2012, via Wang et al. 2015 |
| TC-15 | In a well-mixed population awareness lowers the final size but not the threshold; when awareness arises only near the outbreak, it can stop the disease completely below a threshold infection rate, more so when the communication and contact networks overlap | reported | Funk, Gilad, Watkins, Jansen 2009, *PNAS* 106:6872, https://doi.org/10.1073/pnas.0810762106 (abstract) |
| TC-16 | A small share of committed protectors (5 %) markedly lowers epidemic size | reported | via Wang et al. 2015 |
| TC-17 | On scale-free networks the hubs must protect themselves for the disease to die out | reported | Bagnoli et al., via Wang et al. 2015 |
| TC-18 | Behaviour models sort by the information people use (local or global; prevalence or belief) and by what behaviour changes (disease state, parameters, contact structure) | reported | Funk, Salathé, Jansen 2010, *J. R. Soc. Interface* 7:1247, https://doi.org/10.1098/rsif.2010.0142 (abstract) |
| TC-19 | When people choose contacts by weighing their value against the expected cost of infection, the course of the epidemic and the best policy change significantly | reported | Fenichel et al. 2011, *PNAS* 108:6306, https://doi.org/10.1073/pnas.1011250108 (abstract) |
| TC-20 | Two interacting contagions can produce explosive, first-order outbreaks where small changes in transmissibility flip the outcome, some only on clustered networks | reported | Hébert-Dufresne and Althouse 2015, *PNAS* 112:10551, https://doi.org/10.1073/pnas.1507820112 (abstract) |
| TC-21 | Pair approximations carry systematic bias; on an SIRS random regular network the densities close and the coarse diagram has two stable endemic states between turning points | sourced | Siettos 2010, https://arxiv.org/abs/1012.4194 (partial read); see [EF-16](coarse-layers-from-fine-rules.md) |
| TC-22 | A review of more than 200 studies: a warning's source, channel, consistency, credibility, accuracy, clarity and frequency largely decide whether it is heeded, with the receiver's social setting, family, risk perception and experience | reported | Mileti and Sorensen 1990, ORNL-6609, https://www.osti.gov/biblio/6137387 (abstract) |
| TC-23 | Field evidence contradicts the myths of "keep it simple", "cry wolf", public panic, and unwillingness to respond | reported | Mileti and Sorensen 1990 (abstract) |
| TC-24 | Most warning systems can alert the public given three or more hours; faster warning needs special systems | reported | Mileti and Sorensen 1990 (abstract) |
| TC-25 | Warning response runs hear, understand, believe, personalise, respond, and people seek confirmation at each stage | reported | Mileti and Sorensen 1990, via secondary descriptions |
| TC-26 | Milling: people search for information and confirm a warning with others before acting; fuller content (what, where, when, what to do) shortens it | reported | Wood, Mileti et al. 2018, *Environment and Behavior* 50:535, https://doi.org/10.1177/0013916517709561 |
| TC-27 | The Protective Action Decision Model: exposure, attention and comprehension; then threat, action and stakeholder perceptions; information search when uncertain; situational factors turn the decision into behaviour (see [PB-52](pop-behaviour.md)) | reported | Lindell and Perry 2012, *Risk Analysis* 32:616, https://doi.org/10.1111/j.1539-6924.2011.01647.x |
| TC-28 | In the ignorant–spreader–stifler rumour model about 20.3 % of a well-mixed population never hears the rumour | reported | Daley and Kendall 1964, *Nature* 204:1118, https://doi.org/10.1038/2041118a0 |
| TC-29 | In an online experiment a health behaviour spread farther and faster on clustered networks than on random ones, because adopting it needed reinforcement from several contacts (complex contagion) | reported | Centola 2010, *Science* 329:1194, https://doi.org/10.1126/science.1185231 |
| TC-30 | In about 126,000 rumour cascades, false news was 70 % more likely to be retweeted and the truth took about 6× as long to reach 1,500 people; bots spread true and false alike, so people drive the gap | reported | Vosoughi, Roy, Aral 2018, *Science* 359:1146, https://doi.org/10.1126/science.aap9559 |
| TC-31 | Playing a prebunking game (about 15,000 players) lowered the rated reliability of fake news built on six techniques, most for those who believed it most | reported | Roozenbeek and van der Linden 2019, *Palgrave Commun.* 5:65, https://doi.org/10.1057/s41599-019-0279-9 |
| TC-32 | Inoculation effects lasted at least about a week and faded over about two months without boosters | reported | Maertens, Roozenbeek, Basol, van der Linden 2021, *J. Exp. Psychol. Appl.* 27:1, https://doi.org/10.1037/xap0000315 |
| TC-33 | An infodemic is too much information, including false or misleading information, during an outbreak; it causes confusion and risk-taking and erodes trust in health authorities | reported | Tangcharoensathien et al. 2020, *JMIR* 22:e19659, https://pmc.ncbi.nlm.nih.gov/articles/PMC7332158/ |
| TC-34 | The WHO's crowdsourced framework has 50 actions in five areas, among them verifying evidence, filling information voids and amplifying through trusted messengers | reported | Tangcharoensathien et al. 2020 |
| TC-35 | People whose first warning came from someone they know (a door-knock, a call) acted faster because they did less confirming; at Oroville 2017 (about 500 sampled evacuees) personal warnings beat TV and radio, and very few said social media shaped the decision | sourced | John Sorensen, "Inside Disaster Warnings with Dr. John Sorensen" (YouTube q8cxi-l1pYk, [0:19:49], [0:23:01]) |
| TC-36 | West Helena chemical accident: 87 % of those told to shelter in place evacuated instead, and warning-response factors explained the choice | sourced | John Sorensen, "Inside Disaster Warnings with Dr. John Sorensen" (YouTube q8cxi-l1pYk, [0:17:59]) |
| TC-37 | In a catalogue of warning events, about half the failures were human or behavioural and half were hardware or communication failures | sourced | John Sorensen, "Inside Disaster Warnings with Dr. John Sorensen" (YouTube q8cxi-l1pYk, [0:10:00]) |
| TC-38 | A warning should carry source, hazard, location, guidance and time; complete messages cut the information seeking that delays action, and "keep it short" is wrong for warnings | sourced | John Sorensen, "Inside Disaster Warnings with Dr. John Sorensen" (YouTube q8cxi-l1pYk, [0:13:59], [0:27:01]) |
| TC-39 | Instructional messages need four parts (who is affected, how it spreads, explanation, action); a decade of real messages almost never said who is affected, and dropping any one part made messages fail in tests | sourced | Timothy Sellnow, "The Instructional Dimension of Risk and Crisis Communication" (YouTube iJeEMXaBAgI, [0:22:32], [1:19:57]) |
| TC-40 | People test a message against other sources and people; the same message from several sources convinces (convergence), less once a topic is politicised, and a counter to misinformation works better the sooner it comes | sourced | Timothy Sellnow, "The Instructional Dimension of Risk and Crisis Communication" (YouTube iJeEMXaBAgI, [0:43:29], [1:33:24]) |
| TC-41 | Clustering slows a single contagion but keeps interacting contagions together: with synergistic co-infection, households turn members into super-spreaders | sourced | Laurent Hébert-Dufresne, "Spread of Viruses and Ideas in the Age of Echo Chambers" (YouTube RZGdnAM9Kis, [0:23:11]) |
| TC-42 | In an adaptive-network model of two opposed opinions, an opinion wins fastest by splitting effort evenly between reinforcing its own side and debating the other; echo chambers win only slowly and destroy diversity | sourced | Laurent Hébert-Dufresne, "Spread of Viruses and Ideas in the Age of Echo Chambers" (YouTube RZGdnAM9Kis, [0:30:24]) |
| TC-43 | R = Duration × Opportunities × Transmission × Susceptibility, so cutting any factor cuts R in proportion; the same split applies to online content | sourced | Adam Kucharski, "The Maths of Contagion: Why Things Spread and Why They Stop" (YouTube tdDs13-paDA, [0:23:32]) |
| TC-44 | Popular online content has R near 1–2; it goes viral through a short generation time (hours to about 2 days) and amplification by a few outlets, not through a high R | reported | Adam Kucharski, "The Maths of Contagion: Why Things Spread and Why They Stop" (YouTube tdDs13-paDA, [0:21:00], [0:37:20]) |
| TC-45 | About 80 % of COVID transmission came from 10–20 % of infections, so most introductions fizzle and a few explode | sourced | Adam Kucharski, "The Maths of Contagion: Why Things Spread and Why They Stop" (YouTube tdDs13-paDA, [0:38:23]) |
| TC-46 | A widely shared message mutated in about 1 in 10 reposts; mutations that added a personal touch, detail or a timescale spread better and were selected | reported | Adam Kucharski, "The Maths of Contagion: Why Things Spread and Why They Stop" (YouTube tdDs13-paDA, [0:33:18]) |
| TC-47 | Isolation works only when little transmission comes before visible symptoms (SARS yes, flu no); in a toy agent model a 20 % leak doubled total cases and a 50 % leak was barely better than none | reported | Adam Kucharski, "The Maths of Contagion: Why Things Spread and Why They Stop" (YouTube tdDs13-paDA, [0:48:06]); Grant Sanderson, "Simulating an epidemic" (YouTube gxAaO2rsdIs, [0:07:21]) |
| TC-48 | A push rumour in a fully mixed group with no stopping is an exact counting chain (rate k(n − k)/n): everyone hears, in mean time about 2 ln n; "20 % never hear" (TC-28) needs spreaders who stop | reported | Ayalvadi Ganesh, "Lecture 5: Rumour spread on the complete graph" (YouTube bMOv4iilysk, [0:31:41], [0:35:20]) |
| TC-49 | Compartmental forecasts were good only until measures and behaviour reacted; early detection undercounted about 100-fold (Spain, March 2020) | sourced | Juan José Nieto and David Gómez-Ullate, "Las matemáticas frente a la Covid-19" (YouTube ZZlDwg8cypw, [0:29:25], [0:30:58]) |
| TC-50 | Outrage (fear, anger, misery) drives perceived hazard: expert hazard and outrage correlate about 0.2, perceived hazard and outrage about 0.7 (studies not named) | sourced | Peter Sandman, "Risk = Hazard + Outrage: Risk Communication & a critique of COVID-19 Communication" (YouTube OlGeWC8j-Zo, [0:09:22]) |
| TC-51 | Over-reassurance backfires whether believed or not; confident statements later proved wrong are the main source of mistrust, while stated uncertainty keeps credibility through reversals | sourced | Peter Sandman, "Risk = Hazard + Outrage: Risk Communication & a critique of COVID-19 Communication" (YouTube OlGeWC8j-Zo, [0:33:01]) |
| TC-52 | Fear drives precautions and the misery of the precautions drives people to drop them; officials over-diagnose panic | sourced | Peter Sandman, "Risk = Hazard + Outrage: Risk Communication & a critique of COVID-19 Communication" (YouTube OlGeWC8j-Zo, [0:41:46]) |
| TC-53 | Three delays add up: issuing, spreading and acting on a warning; family and friends gave the fastest first alert; a single phone-alert channel reached about 18 % fast and peaked near 30 %; several channels reach faster and further | sourced | Dennis Mileti, "PrepTalks: Modernizing Public Warning Messaging" (YouTube oYya009bc2M, [0:08:09]) |
| TC-54 | Milling is the norm; very short alerts did nothing; content is the strongest factor (source first, then hazard, location, who, consequences, action, when, how it helps, expiry); without a stated location people assume they are safe | sourced | Dennis Mileti, "PrepTalks: Modernizing Public Warning Messaging" (YouTube oYya009bc2M, [0:14:05]) |
| TC-55 | One false alarm that is explained in the next warning raises vigilance; only repeated false alarms erode response; conflicting mandatory and voluntary orders from overlapping authorities confuse people | sourced | Dennis Mileti, "PrepTalks: Modernizing Public Warning Messaging" (YouTube oYya009bc2M, [0:22:19]) |
| TC-56 | When the healthy rewire away from infected contacts, even small rewiring makes extinction and an endemic state coexist (bistability) and faster rewiring causes extinction | sourced | Ira Schwartz, "Fear in Networks: How social adaptation controls epidemic outbreaks" (YouTube tTSnX-Liro0, [0:13:35]) |
| TC-57 | Fade-out is a finite-population rare event: the mean time to extinction grows exponentially with population size, and fluctuations grow near the turning point | sourced | Ira Schwartz, "Fear in Networks: How social adaptation controls epidemic outbreaks" (YouTube tTSnX-Liro0, [0:05:45]) |
| TC-58 | Two contagious fears, of the disease and of the remedy, produce cycles of vigilance and complacency | sourced | Joshua Epstein, "Incorporating Human Behaviours in Epidemic Modelling Panel Discussion" (YouTube PgAPQUkTZfw, [0:26:09]) |
| TC-59 | People depart from directives when prevalence falls even though the directives stay; perceived prevalence depends on trust in official reporting | reported | Joshua Epstein, "Incorporating Human Behaviours in Epidemic Modelling Panel Discussion" (YouTube PgAPQUkTZfw, [0:13:21]) |
| TC-60 | Averaging time-varying behaviour into constant coefficients makes models over-project peaks; with self-regulating behaviour built in, one group called a peak a month ahead | sourced | Michael Li, "Incorporating Human Behaviours in Epidemic Modelling Panel Discussion" (YouTube PgAPQUkTZfw, [0:07:10]) |
| TC-61 | Prebunking by 30–90 s video ads raised recognition of manipulation by about 5 % in the field, far below lab results; active inoculation (a game) beats passive; effects fade without boosters and hold about 3 months with them | sourced | Sander van der Linden, "Psychological inoculation against misinformation" (YouTube oJ2l48cNEJQ, [0:45:33]) |
| TC-62 | False news was 70 % more likely to be shared although its spreaders had fewer followers; novelty relative to what the hearer saw in the previous 60 days, and surprise and disgust, explain it; bots sped true and false news equally | sourced | Sinan Aral, "How we can protect truth in the age of misinformation" (YouTube -7ORAKULel4, [0:04:36]) |
| TC-63 | Information voids (questions with no credible answer available) come before rumours; filling them in advance beats chasing rumours; overload makes people avoid information | reported | Tina Purnat, "What we have learned about Infodemic Management Since the Beginning of COVID-19" (YouTube 8aH3m8TRQwQ, [0:03:45]) |
| TC-64 | When a hoax broadcast by a trusted station (Radio Quito, 1949) was exposed, the crowd turned on the source and burned the building | reported | Verónica Yépez and Cristian Espinosa, "Infodemia: desinformación en crisis sanitaria" (YouTube CcXnuehe-uc, [0:35:00]) |

Already in PB and not repeated here: panic and looting are rare (PB-36, PB-49); mutual aid is the norm (PB-38); people move towards their attachments (PB-40); delay and milling are the real failures (PB-52, PB-53, PB-55); evacuation orders raise leaving but do not force it (PB-51).

## What the talks add

Read in full from 15 talks; rows TC-35 to TC-64. They change four rules above and add five.

- **Rule 4, the pipeline:**
  - How fast a pop acts depends on how close the source is and how complete the message is, not only on confidence (TC-35, TC-38, TC-54).
  - A warning from someone the pop knows skips most milling.
  - A broadcast's milling time shrinks with each content element it carries (source, hazard, **location**, action, time).
  - A broadcast with no location fails the "personalise" step: pops assume they are safe (TC-54).
- **Rule 8, prebunking:** in the field, a broadcast prebunk is weak (about 5 %) and fades (TC-61). Set a broadcast prebunk to s ≈ 0.05; a pop taught in person (active) gets s ≈ 0.3; both fade over about 3 months unless renewed.
- **Rule 9, the levers:**
  - Orders **advise**; the pop's own model picks the action: 87 % sheltering-in-place orders became evacuations (TC-36).
  - **One broadcast channel reaches at most about 30 % of its audience** (TC-53); several channels add up, and household telling is the fastest path.
  - About half of warning failures are hardware (TC-37), so comms fail in the world, not only by order.
  - A source loses credibility when its confident claims are contradicted (TC-51). Hedged messages keep credibility through reversals, and one explained false alarm costs nothing (TC-55). A cover-up that is exposed can turn people on the source (TC-64).
- **Rule 7, rumours:**
  - Novelty is measured against what the hearer already believes (TC-62).
  - Silence creates information voids that spawn rumours (TC-63): when a district has questions and no credible source, rumours start there.
  - Claims mutate as they are retold, and the variants that add a personal detail or a time spread better (TC-46). This gives spec 39's per-hop drift a direction.
- **New: spreading needs a stopping rule** (TC-48, TC-28). Without stifling everyone hears; with it about 20 % never do. Spec 39 must say when a pop stops telling, for example after telling someone who already knew. The no-stopping case is an exact counting chain, which gives spec 40 an exact jump form.
- **New: information wins by speed, not reach** (TC-43, TC-44). Its generation time is hours; the bite-to-turn time is 1–2 days (decision 0014). The coarse form must keep the information step shorter than the disease step (EF rule 3).
- **New: contacts are overdispersed** (TC-45). Spec 38's binomial contacts give every zombie the same expected contacts; real spread comes from a few. A negative-binomial contact count gives "most introductions fizzle, a few explode" and lowers the take-off share at the same mean.
- **New: the hidden bite decides whether any response works** (TC-47). Spec 24 rule 7's secret bite is the share of spread before visible symptoms. Warnings, cover-ups and compliance act on it. An A3 test must sweep it, because its effect is steeply non-linear.
- **New: behaviour wears off** (TC-52, TC-58, TC-59, TC-60):
  - precautions are dropped as local prevalence falls, even when the orders stand;
  - fear of the player's own measures can spread as a second fear;
  - pops avoid the known-bitten specifically, not everyone equally (TC-56), which can make a district bistable.

  The coarse layer must carry the behaviour state (u, h, a), never a constant compliance (TC-60).

## What this means for us (Proposal)

All rules are `estimate`, tied to the findings by ID; nothing here is decided. "Covered" means spec 38, spec 39 or PB already says it.

1. **`warning` becomes a per-pop belief** (covered as a recommendation in specs 38 and 39; made concrete here). The encounter reads the met person's `w = stage weight × confidence`, with stage weight 0 (not heard), 0.2 (heard, not acting; a placeholder) or 1 (acting). A region's `warning` at Macro is the **mean over encounters**, not over the population, because awareness forms where the zombies are (TC-14, TC-15). That weighting is new.
2. **The disease writes information** (new). Every encounter writes a `Seen` belief (hop 0, confidence 1) to the person met, if they live, and to the living who see it: line of sight at Individual, a measured number of witnesses per encounter at City and Macro (TC-1, TC-8). Spec 39 defines `Seen` but nothing produces it yet. A `Told` or `Broadcast` claim never lowers a `Seen` belief ([0048](../decisions/0048-realistic-time-default-commit-on-observation-and-pins.md)).
3. **Forgetting** (new in effect). Spec 39 keeps confidence × 0.7 per hop; add decay with age at a rate δ unless refreshed by a new sighting or telling. Without forgetting the coupling vanishes (TC-5) and second waves cannot happen (TC-10).
4. **The pop's response pipeline** (partly covered by PB-52, PB-53): **hear** (a belief exists) → **believe** (confidence ≥ 0.5, so a fresh broadcast passes and a three-hop rumour does not) → **personalise** (it concerns the pop's own district or household) → **mill** (wait for a second independent source, or time out) → **act** (arm, barricade, avoid strangers, leave, through PB's obedience rule). Hearing needs one source and acting needs two: a simple contagion feeding a complex one (TC-25, TC-26, TC-29). Test: word of mouth alone leaves 10–30 % who never hear (TC-28).
5. **Fear is a third, faster contagion** (new; PB treats fear only as a personal state). Zombies, the bitten and frightened pops all spread it, so it leads the disease (TC-9). Hiding cuts incidence; fleeing spreads it (TC-11, TC-12). Flight stays rare (PB), but **a fleer hiding a bite is how the outbreak jumps between districts**.
6. **Local beats global, and communications decide whether information matters at all** (new; the main design consequence). Word of mouth on the same household and neighbour network as the encounters can stop a take-off; a broadcast that reacts to prevalence only shrinks the outbreak; a standing alert issued before take-off can push r0 under 1 (TC-14, TC-15). Awareness must itself take off on the communication graph (TC-5); cutting communications raises that bar.
7. **Rumours are competing claims** (new). A false claim ("the bitten recover", "the army is coming") is a belief about the same thing with a different value. Its telling rate gets a novelty factor (placeholder 1.7, from TC-30). A pop holds the claim with the higher confidence, and `Seen` always wins.
8. **Prebunking is an information vaccine** (new). A `Prebunk` order gives its audience a resistance s (placeholder 0.3 when taught in person, 0.05 by broadcast; see TC-61) that scales down confidence in later false claims and fades over weeks (TC-31, TC-32).
9. **The player's levers are orders traced through spec 39** (mostly new): `Broadcast` (needs comms; confidence = the source's credibility, TC-22), seeding a rumour (a `Tell`, exists), `Cover-up` (blocks broadcasts about the threat, may broadcast false reassurance), `Prebunk`, and `SetComms` (exists). `Belief` gains a `cause` event id, so the cause trace walks from a destroyed zombie back to the broadcast that warned its killer. A source loses credibility when what pops see contradicts it (a cover-up found out), not after an explained false alarm (TC-23).
10. **Panic stays rare** (covered). PB §7's bands must still pass with this layer on; milling, not panic, remains the main failure.

## The coarse form

At Macro the living are split by disease (spec 38: susceptible, exposed) and by information stage: **u** (not heard), **h** (heard, milling) and **a** (acting). The day stays spec 38's keyed-binomial tau-leap, in this order:

1. **Disease:** spec 38 rule 4, with the encounter using W, the encounter-weighted mean warning. Spec 38's encounter is linear in `w`, so W is the only information number the disease needs: `r0 = convert × contacts × bite × (1 − 0.7W) ÷ (kill × W)`. The outbreak fizzles when W exceeds W\* = `convert·contacts·bite ÷ (kill + 0.7·convert·contacts·bite)`, a readout of how many people must be acting here.
2. **Sightings:** u and h move to a in proportion to encounters × witnesses per encounter.
3. **Broadcast:** with comms up, u moves to h with probability reach × credibility.
4. **Telling:** u moves to h with `1 − exp(−τ·(h + a)/N)` per day (simple contagion).
5. **Confirmation:** h moves to a by a second source or a timeout (complex contagion).
6. **Forgetting:** a to h and h to u at rate δ.
7. **Flight:** fleeing exposed pops move along the travel matrix.

Between districts, Granell's Markov chain (TC-3) runs with districts as nodes and two matrices (travel and communications); its check λ·Λ_max > δ (TC-5) says whether information can take off at all.

**Measured from short entity runs, never set** ([EF](coarse-layers-from-fine-rules.md)): the ratio φ of warning among those met to warning in the population, witnesses per encounter, tellings per pop per day, milling time and δ. **φ is the crux.** A well-mixed aggregate that assumes φ = 1 predicts take-off exactly where the entity run fizzles.

**What must agree with the entity run:**
- **A2:** infected-ever distributions, means and take-off shares in five regimes (no information, word of mouth only, a broadcast on day d, comms down, a cover-up); u, h and a on days 1, 3 and 7; the share who never hear.
- **A3:** the take-off transition against telling rate falls within one grid step and moves the same way when comms are cut; every lever acts in the same direction; second waves appear in both when forgetting is on. Tests start from both sides of any transition, since these systems can be bistable or explosive (TC-20, TC-21).

## Open questions

Carried to [world simulation](../design/world-simulation.md) and [layer jumps](../design/layer-jumps.md):

- Is fear a separate contagion in spec 24, or only an input to the existing fear state with a social term?
- Which levers does the first build give the player: broadcast and comms only, or also rumour, cover-up and prebunk?

## Sources

- Granell, C., Gómez, S., Arenas, A. (2013). Dynamical interplay between awareness and epidemic spreading in multiplex networks. *PRL* 111:128701. arXiv 1306.4136
- Epstein, J. M., Parker, J., Cummings, D., Hammond, R. A. (2008). Coupled contagion dynamics of fear and disease. *PLoS ONE* 3:e3955
- Wang, Z., Andrews, M. A., Wu, Z.-X., Wang, L., Bauch, C. T. (2015). Coupled disease–behavior dynamics on complex networks: a review. *Phys. Life Rev.* 15:1–29
- Funk, S., Gilad, E., Watkins, C., Jansen, V. A. A. (2009). The spread of awareness and its impact on epidemic outbreaks. *PNAS* 106:6872
- Funk, S., Salathé, M., Jansen, V. A. A. (2010). Modelling the influence of human behaviour on the spread of infectious diseases: a review. *J. R. Soc. Interface* 7:1247
- Fenichel, E. P., et al. (2011). Adaptive human behavior in epidemiological models. *PNAS* 108:6306
- Hébert-Dufresne, L., Althouse, B. M. (2015). Complex dynamics of synergistic coinfections on realistically clustered networks. *PNAS* 112:10551
- Siettos, C. (2010). arXiv 1012.4194
- Mileti, D. S., Sorensen, J. H. (1990). Communication of emergency public warnings. ORNL-6609
- Wood, M. M., Mileti, D. S., et al. (2018). Milling and public warnings. *Environment and Behavior* 50:535
- Lindell, M. K., Perry, R. W. (2012). The Protective Action Decision Model. *Risk Analysis* 32:616
- Daley, D. J., Kendall, D. G. (1964). Epidemics and rumours. *Nature* 204:1118
- Centola, D. (2010). The spread of behavior in an online social network experiment. *Science* 329:1194
- Vosoughi, S., Roy, D., Aral, S. (2018). The spread of true and false news online. *Science* 359:1146
- Roozenbeek, J., van der Linden, S. (2019). Fake news game confers psychological resistance against online misinformation. *Palgrave Commun.* 5:65
- Maertens, R., Roozenbeek, J., Basol, M., van der Linden, S. (2021). *J. Exp. Psychol. Appl.* 27:1
- Tangcharoensathien, V., et al. (2020). Framework for managing the COVID-19 infodemic. *JMIR* 22:e19659
