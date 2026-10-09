# City layout generation: streets, districts, blocks and plots

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [world systems](../design/world-systems.md), [content tools](../design/content-tools.md), [zoom continuum](../design/zoom-continuum.md), [spec 51](../tier0/specs/51-dwellings.md). **Evidence:** read in full or in the relevant sections (`sourced`): Chen et al. 2008 (tensor-field streets), Vanegas et al. 2012 (parcels), Aliaga et al. 2008 (example-based layout), Angel et al. 2005 Lincoln working paper (density decline), Boeing 2020 (street networks of every urban area) and Boeing 2017 (27,000 US networks), the UN-Habitat five-principles note (2014), Martin Evans' road generation blog post (2015), and Wikipedia pages on the Commissioners' Plan of 1811, the Cerdà plan (Spanish page), street hierarchy, machiya, the Public Land Survey System, Radburn and space syntax. From search summaries only (`reported`): Parish and Mueller 2001, Kelly and McCabe 2006, Lechner et al. 2003, SLEUTH, Watabou's generators. Not read: the Atlas of Urban Expansion volume 2 (212 MB PDF, not downloaded) and the "State of the Streets" working paper, so the Atlas road-share and block-size trends for 1990-2015 are missing; the search budget ran out before they could be found another way. **IDs:** CL.

The town generator ([content tools](../design/content-tools.md) §2, [spec 51](../tier0/specs/51-dwellings.md)) makes households, people and businesses with addresses, but no streets, blocks or plots in space. This note gathers the methods and the urban-form numbers for the next step down: from a town record to districts, streets, blocks and plots that agree with it. Settlement placement, city sizes and inter-town roads are in [world-numbers-settlement](world-numbers-settlement.md) (WNS) and are only cited here.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| CL-1 | Parish and Mueller (SIGGRAPH 2001) grow highways and streets with an extended L-system; input image maps (land and water, population density) act as global goals, and local constraints (water, parks, existing roads) adjust or reject each proposed segment; land between streets is then cut into lots and buildings are made by a second L-system | reported | history.siggraph.org/?p=116923 (abstract); Purdue SG2011 course "urban layouts" |
| CL-2 | A later critique of Parish and Mueller: buildings did not differ by use, so whole cities looked like downtown Manhattan; their simple lot step left odd empty areas inside blocks and discarded lots with no street access | reported | ccl.northwestern.edu midgraph.cities.pdf (search summary); Vanegas et al. 2012 §2 |
| CL-3 | Chen et al. 2008 trace streets as hyperstreamlines of a 2D symmetric tensor field, whose two eigenvector families are always perpendicular; they argue real street patterns have two dominant directions, which is why the tensor field fits | sourced | Chen, Esch, Wonka, Mueller, Zhang, ACM TOG 27(3) 103, 2008 |
| CL-4 | Chen's basis fields: grid (one direction), radial (centre element, circles and spokes), boundary (from water or park outlines, so a major road follows a coastline) and height field (roads follow contours); fields are blended with decaying radial basis functions and can be rotated by Perlin-noise fields for irregular towns | sourced | same as CL-3, §5 |
| CL-5 | Chen's inputs are four 512 x 512 maps: water, parks and forest, height and population density; density sets the streamline separation dsep, so streets thin out towards the city edge | sourced | same as CL-3, §3 and §6.1 |
| CL-6 | Chen traces major and minor streamlines in turn from seeds dsep apart (independent tracing left a disconnected graph); a streamline stops at the domain edge, a degenerate point, a loop, a maximum length or closer than dsep to another, then looks ahead a distance to join an existing street; seeds are prioritised by closeness to water, degenerate points and population centres; narrow water may be bridged | sourced | same as CL-3, §6.1 |
| CL-7 | Chen builds a two-level hierarchy: major roads first; their edges split the land into regions, each with its own (possibly discontinuous) tensor field for minor roads, so neighbourhoods can differ in orientation | sourced | same as CL-3, §6.2 |
| CL-8 | Martin Evans implemented tensor-field roads for a game: four field types (grid, radial, height-following, polyline), distance-weighted blending, RK4 tracing with a 90 degree sign fix, quadtrees for vertices and segments, seed priority from a density field; 5 to 10 s per city; he names error build-up along streamlines as the weak point; determinism is not discussed | sourced | martindevans.me, "Procedural Generation For Dummies: Roads", 2015-12-11 |
| CL-9 | Vanegas et al. 2012 subdivide each block into parcels with two styles: straight-skeleton (every parcel fronts the street, rear sides touch other parcels, like perimeter blocks) and recursive OBB splits (quadrilateral parcels, split across the longest side of the minimum-area oriented box) | sourced | Vanegas, Kelly, Weber, Halatsch, Aliaga, Mueller, CGF 31(2), 2012 |
| CL-10 | Vanegas' per-block attributes: parcel area bounds Amin, Amax; frontage width bounds Wmin, Wmax; split irregularity in 0 to 1; and a street-access preference lambda in 0 to 1 (at 1 every parcel has street access, by turning a split 90 degrees when a child would be landlocked) | sourced | same as CL-9, §4 |
| CL-11 | Vanegas' guarantees: parcels partition the block with no unused space, have simple near-rectangular outlines, and small or triangular leftovers are merged with neighbours until above Amin | sourced | same as CL-9, §4 |
| CL-12 | Vanegas calibrate from real GIS parcels: per block they take mean and standard deviation of parcel area and frontage and set Amin, Amax = mean -/+ 2 SD (same for W); histograms of generated and real parcel area and aspect ratio then match | sourced | same as CL-9, §5 |
| CL-13 | Vanegas: a whole city's parcels generate in under 3 s; one block re-subdivides in 1 to 10 ms; OBB subdivision is local, so editing part of a block changes only nearby parcels | sourced | same as CL-9, §5 |
| CL-14 | Aliaga, Vanegas and Benes 2008 synthesise new layouts from example fragments of real cities: street intersections carry attributes of the nearby street and parcel geometry, new streets join those points, parcels are generated in the new blocks; operations are join, expand and blend | sourced | ACM TOG 27(5) 160, 2008 |
| CL-15 | Kelly and McCabe's survey (ITB Journal 14, 2006) reviews procedural city methods (L-systems, grid and template layouts, agent-based growth); their CityGen system builds primary roads first, then secondary roads, then buildings, as an interactive pipeline | reported | ITB Journal 14 (2006), citations only; history.siggraph.org/?p=142350 |
| CL-16 | Lechner, Watson, Wilensky and Felsen (2003) grow a city with agents in NetLogo from terrain alone: developer agents lay roads and place residential, commercial and industrial land over simulated time; a later version adds parks, building age and density and a painting interface | reported | ccl.northwestern.edu cities.tr.pdf and TR-2007-33 (search summaries; the site returned 403) |
| CL-17 | SLEUTH (Clarke) is a cellular automaton of urban growth with four rules (spontaneous, new spreading centres, edge growth, road-influenced) and five coefficients (dispersion, breed, spread, slope, road gravity); inputs are slope, land cover, exclusion, urban extent, transport and hillshade; it is calibrated by brute force against historical maps and run as Monte Carlo | reported | sciencebase.gov SAMBI urban growth; woodwellclimate.org Jantz et al.; search summaries |
| CL-18 | Watabou's Medieval Fantasy City Generator splits the town into Voronoi cells ("patches"), gives each a ward type (market, gate, craftsmen, merchant, patriciate, slum, castle, temple), runs main streets from gates to the centre, wraps a wall with gates and fills wards with lots and buildings; the author says the goal is a good-looking map, not an accurate model | reported | watabou.itch.io/medieval-fantasy-city-generator; metafilter.com/167243; groupfinder.eu summary |
| CL-19 | Watabou's generator source (an older version) is public on GitHub under GPL-3.0, written in Haxe with OpenFL; it lacks water bodies and some newer features | sourced | github.com/watabou/TownGeneratorOS (README, licence) |
| CL-20 | Space syntax measures street networks by topology: integration (how few turns reach every other segment, optionally within a radius) and choice (how many shortest paths pass through); integration is argued to predict pedestrian use and choice car flow; critics (Ratti 2004) note it favours long straight streets | reported | en.wikipedia.org/wiki/Space_syntax (secondary) |
| CL-21 | Manhattan's 1811 grid: cross streets 60 ft (18 m) wide about 200 ft (61 m) apart, giving about 20 blocks per mile; avenues 100 ft (30 m); blocks between avenues 610 to 920 ft (190 to 280 m), 800 ft (240 m) on the west side; standard lots 20 or 25 ft by 100 ft (6 to 7.6 m by 30 m); about 2,000 blocks | sourced | en.wikipedia.org/wiki/Commissioners'_Plan_of_1811 |
| CL-22 | Barcelona's Cerdà plan (1859): square blocks of 113.3 m with chamfered corners, about 1.24 ha each, 420 blocks; streets 20 m (10 m roadway, 5 m pavements), 30 m and 60 m; buildings on two sides only, about 50 % of the block left as garden, which grew to about 90 % built; height up to 16 m, later about 20 m | sourced | es.wikipedia.org/wiki/Plan_Cerdá (the chamfer is given as 15 m and as 20 m on the same page) |
| CL-23 | Manhattan's 1811 grid puts about 31 % of land in streets (a 61 x 240 m block in a 79 x 270 m cell) and has about 47 intersections per km2 | derived | CL-21 |
| CL-24 | Cerdà's grid puts about 30 % of land in streets (1.24 ha block in a 133.3 m square cell, 1.78 ha) and has about 56 intersections per km2 | derived | CL-22 |
| CL-25 | UN-Habitat's five principles (2014): streets at least 30 % of land and at least 18 km of street per km2; at least 15,000 people per km2 (150 per ha); at least 40 % of floor space in economic use; 20 to 50 % of residential floor area low-cost; single-use blocks under 10 % of a neighbourhood | sourced | UN-Habitat, "A New Strategy of Sustainable Neighbourhood Planning: Five Principles", Discussion Note 3, 2014 |
| CL-26 | The same note says 20 to 30 % of urban land and 40 to 60 % of commercial-centre land goes to roads and parking, and recommends 15 to 20 % for open public space; its model grid of 9 by 9 streets in 1 km2 has streets 111 m apart and 18 km of street | sourced | same as CL-25 |
| CL-27 | 18 km of street per km2 at 30 % of land means an average street reserve of about 16.7 m; the 111 m grid gives 81 intersections per km2 | derived | CL-25, CL-26 |
| CL-28 | A 1950s CBD study, quoted by UN-Habitat, puts the best CBD floor split at 68 % economic, 22 % residential and 10 % public; UN-Habitat suggests 40 to 60 % economic, 30 to 50 % residential and 10 % public for a mixed neighbourhood | sourced | same as CL-25, Principle 3 |
| CL-29 | Built-up densities from the UN-Habitat note's table: Dhaka 55,530, Hong Kong 53,050, Mumbai 43,570, Milan 27,380, Casablanca 26,280, Cairo 22,990, Seoul 20,600 people per km2 of built-up area | sourced | same as CL-25, Figure 4 |
| CL-30 | Angel et al. (120 cities, 2000): average built-up density 28 +/- 5 people per ha in land-rich developed countries, 70 +/- 8 in other developed countries, 135 +/- 11 in developing countries | sourced | Angel, Parent, Civco and Blei, "The Persistent Decline in Urban Densities", Lincoln Institute working paper (2010) |
| CL-31 | Built-up densities fell 2.0 +/- 0.4 % a year from 1990 to 2000 in that sample; in 20 US cities they fell 1.9 % a year from 1910 to 2000; in 30 cities over 1800 to 2000 density rose in the 19th century, peaked around 1890, then fell about threefold at 1.0 to 1.5 % a year | sourced | same as CL-30 |
| CL-32 | Across the 120 cities, a 10 % rise in national GDP per capita goes with 2.3 to 4.2 % lower density; 10 % more city population with 1.8 to 1.9 % higher density; 10 % more arable land per capita with 2.0 to 3.0 % lower; 10 % higher Gini with 5.6 to 9.0 % lower; 10 % more buildable land at the edge with 3.4 to 5.6 % lower | sourced | same as CL-30, §4 |
| CL-33 | Historical densities: Paris in 1800 about 500 people per ha on 11 km2, edge about 2 km from the centre; New York's Tenth Ward in the 1870s about 700 per ha; central Dharavi up to 3,300 per ha | sourced | same as CL-30, §2 and §6 |
| CL-34 | Density depends on the boundary: for Bandung 1991, built-up density 274, urbanized-area density (including enclosed open space) 215 and city-footprint density 135 people per ha; Paris 2000 density falls about 9.5 % per km from the centre | sourced | same as CL-30, §2 |
| CL-35 | Boeing 2020, street networks of every urban area (GHS urban centres): population-weighted intersection density per km2 of built-up area ranges from 39 (Eastern Europe) and 46 (Eastern Asia, likely undercounted) to 53 (Northern America), 73 (Western Europe), 97 (Northern Europe), 116 (Southern Europe), 134 (South-Central Asia), 198 (South America), 267 (Eastern Africa) and 284 (Northern Africa) | sourced | Boeing, "Street Network Models and Indicators for Every Urban Area in the World", arXiv 2009.09106, Table 2 |
| CL-36 | Same study: median street segment length ranges from 50 m (Northern Africa) and 63 m (Central America, Southern Europe) to 100 m (Northern America), 139 m (South-Central Asia) and 169 m (Eastern Asia); circuity is 3.7 to 8.6 % above straight lines; average node degree 2.5 (Northern Europe) to 3.0 (South America, Eastern Asia) | sourced | same as CL-35 |
| CL-37 | Orientation order (how grid-aligned streets are, 0 to 1) is highest in Northern America (0.32) and Eastern Asia (0.22) and lowest in Northern Europe (0.02), Western Europe (0.03) and Southern Europe (0.05) | sourced | same as CL-35 |
| CL-38 | Street length per person averages 2.13 m across urban areas and rises by about 1.13 m per USD 10,000 of GDP per capita | sourced | same as CL-35 |
| CL-39 | US urbanized areas (Boeing 2017): typical average segment length 160 m (proxy for block size; 125 m in Portland, 226 m in Danbury); 26 intersections per km2; 2.8 streets per node; nodes are 18 % four-way, 59 % three-way and 21 % dead ends (over 30 % dead ends in Atlanta and Chattanooga) | sourced | Boeing, "A Multi-Scale Analysis of 27,000 Urban Street Networks", arXiv 1705.02198 |
| CL-40 | Among large US metros, street density runs from 6.1 km per km2 (Cincinnati) to 10.6 (Miami, Los Angeles); circuity falls as the share of four-way intersections rises (r2 = 0.43) | sourced | same as CL-39 |
| CL-41 | Street classes from cul-de-sac to local, collector and arterial; arterial access every 90 to 200 m at about 40 km/h on two lanes, every 200 to 400 m at about 60 km/h on four to six lanes; Pompeii's streets were 12.2, 6 and 4.5 m; Tunis medina arterials 3.2 to 3.5 m and dead-end alleys about 1.9 m | sourced | en.wikipedia.org/wiki/Street_hierarchy (secondary) |
| CL-42 | The US Public Land Survey grid is 1 mile (1.6 km) sections of 640 acres; roads follow section lines, so growing towns got arterials a mile apart, sometimes half a mile in postwar suburbs | sourced | en.wikipedia.org/wiki/Public_Land_Survey_System |
| CL-43 | Radburn, New Jersey (1929): superblocks with cul-de-sacs and paths separated from roads; 149 acres with 23 acres of interior park, 670 families and about 3,100 people | sourced | en.wikipedia.org/wiki/Radburn,_New_Jersey |
| CL-44 | Radburn's gross density is about 11 households and 51 people per ha, with 15 % of land in interior parks | derived | CL-43 |
| CL-45 | Kyoto machiya plots are about 5.4 to 6 m wide and about 20 m deep ("eel's beds"), about 110 to 120 m2 | sourced | en.wikipedia.org/wiki/Machiya |
| CL-46 | US new single-family homes sold in 2025: median floor area 2,194 sq ft (204 m2); 41 % of homes built for sale had lots under 7,000 sq ft (650 m2) | sourced | census.gov/construction/chars/highlights.html |
| CL-47 | A Manhattan 25 x 100 ft lot is about 232 m2; a US lot of 7,000 sq ft is about 650 m2, nearly 3 times larger and 5 times a machiya plot | derived | CL-21, CL-45, CL-46 |
| CL-48 | Churchill Gardens, London (council flats): 142 dwellings per ha on 12.5 ha; a UN-Habitat example of four phases runs 93 to 199 dwellings per ha | sourced | same as CL-25, case boxes |
| CL-49 | Berlin's 1925 zoning used nine zone types with floor area ratio limits from 0.2 to 3; New York set citywide FAR rules in 1961; FAR is total floor area over plot area and lot coverage is footprint over plot area | sourced | en.wikipedia.org/wiki/Floor_area_ratio |
| CL-50 | The Spanish Laws of the Indies prescribed a central plaza with straight streets in a grid, oriented against prevailing winds, and polluting trades placed away from the centre | reported | en.wikipedia.org/wiki/Laws_of_the_Indies (secondary; no dimensions given) |
| CL-51 | Grid spacing sets intersection density: a grid of a by b metre cells has 10^6 / (a b) intersections per km2, so 80 to 100 per km2 needs cells about 100 to 110 m, and the US average of 26 per km2 means cells near 200 m | derived | CL-26, CL-39 |
| CL-52 | A town of 3,000 people covers about 107 ha at 28 per ha (land-rich rich country), 43 ha at 70 per ha and 22 ha at 135 per ha (developing); at 2.13 to 3 m of street per person it has 6 to 9 km of street | derived | CL-30, CL-38 |
| CL-53 | Fit for us: tensor fields (CL-3 to CL-8) give controllable patterns but need care with floating point tracing for determinism; L-system growth (CL-1) and agents or cellular automata (CL-16, CL-17) need simulated time and are order-sensitive; example-based synthesis (CL-14) needs real parcel data with licence questions; block-local parcel subdivision (CL-9 to CL-13) is local, fast and calibratable, so it fits lazy generation best | estimate | this note |

## Methods compared

| Method | What it gives | Determinism and locality | Coherence with records | Fit |
| --- | --- | --- | --- | --- |
| L-system growth (CL-1) | Organic or grid networks grown from a seed road | Deterministic with a seed, but each segment depends on all earlier ones; one change ripples | Density map steers growth; no count control | For organic old towns, with a fixed step order |
| Tensor fields (CL-3 to CL-8) | Grid, radial, coast-following and mixed patterns, two-level hierarchy | Field is a pure function; tracing is sequential and float-sensitive; snap to the 0.5 m grid | Density sets street spacing | Main road pattern for planned and modern towns |
| Voronoi wards (CL-18) | Districts with types, gates and a centre | Simple and stable | Ward types map to land use; sized by demand | District layer for any era; fine for small towns |
| Parcel subdivision (CL-9 to CL-13) | Plots with street access, calibrated sizes | Block-local, milliseconds per block | Amin, Amax, Wmin, Wmax per land use class | The plot step |
| Example-based (CL-14) | Real-looking fabric | Depends on the examples | Hard to match exact counts | Later, if a licence-safe parcel set exists |
| Agents and CA (CL-16, CL-17) | Growth history, land use by process | Order-sensitive, needs many steps | Good for history, weak for exact counts | City growth in play (zoning, demand), not world creation |
| Space syntax (CL-20) | A measure, not a generator | Pure graph metric | Where shops go (integrated streets) | Scoring commercial frontage and a test |

## Urban form numbers for calibration

| Quantity | Value | IDs |
| --- | --- | --- |
| Share of land in streets | 20 to 30 % typical urban land, about 30 % in classic grids, 40 to 60 % with parking in commercial centres | CL-23, CL-24, CL-26 |
| Street length per km2 of built-up area | 6 to 11 km in US metros; UN-Habitat target 18 | CL-25, CL-40 |
| Intersections per km2 | 26 (US typical), 39 to 73 (E. Europe, N. America, W. Europe), 97 to 134 (N. and S. Europe, S.-C. Asia), 160 to 284 (Latin America, Africa) | CL-35, CL-39 |
| Median street segment | 50 to 100 m most regions, 139 to 169 m in South-Central and East Asia, 160 m average in the US | CL-36, CL-39 |
| Node mix | US: 18 % four-way, 59 % three-way, 21 % dead ends | CL-39 |
| Block sizes | Manhattan 61 x 190 to 280 m; Eixample 113 m square; US arterials 800 to 1,600 m apart | CL-21, CL-22, CL-42 |
| Street widths | 4.5 to 12 m (Roman), 18 m (Manhattan street), 20 m (Eixample), 30 m avenues, up to 60 m boulevards; alleys about 2 m in medinas | CL-21, CL-22, CL-41 |
| Built-up density | 28 / 70 / 135 people per ha by country group (2000); 200 to 550 per ha in the densest megacities; 500 per ha pre-industrial Paris | CL-29, CL-30, CL-33 |
| Density by income | -2.3 to -4.2 % per +10 % GDP per capita | CL-32 |
| Lots | Machiya 110 to 120 m2; Manhattan 232 m2; US new homes often over 650 m2 | CL-45, CL-46, CL-47 |
| Dwellings per ha | 11 (Radburn gross), 93 to 199 (London estates) | CL-44, CL-48 |
| Floor mix | 40 to 60 % economic, 30 to 50 % residential, 10 % public in a mixed centre | CL-28 |

## What this means for us (Proposal)

Everything here is `estimate`, tied to the findings above. Labels as in CLAUDE.md: nothing below is Decided.

### Pipeline: from a town record to plots

The town record already holds people, households (size, income group), businesses and institutions (type, staff) and a culture ([content tools](../design/content-tools.md) §1, rule 6: places before people). The layout must hold exactly what the record holds ([world simulation](../design/world-simulation.md) §12, [spec 51](../tier0/specs/51-dwellings.md)).

1. **Land budget (from the record, before geometry).** Built-up area = people / density, with density from the country group and income (CL-30, CL-32), era (CL-31, CL-33) and size (CL-32). Split it: streets 25 to 35 % (CL-23, CL-24, CL-26), open space 10 to 20 % (CL-26), residential from households x lot area per income and dwelling type, commercial and industrial from business floor area / FAR (CL-49), public from institutions. The parts must sum to the area within a tolerance; if they do not, density moves inside its band, not the counts.
2. **Anchors from the country.** The country's route graph (WNS-31) gives the edges that reach the settlement footprint. Where each route crosses the footprint boundary is a gate. The centre is the route junction nearest the footprint centroid (a plaza, market or station). Rail stations, ports and river crossings are anchors too. Arterials are the routes themselves, drawn through town between gates and centre on terrain cost, so the town's street graph contains the country's routes as edges by construction.
3. **Districts.** Weighted Voronoi cells around anchor-seeded points (CL-18), sized by the land budget, typed by bid rent: commercial along arterials near the centre (also the most integrated streets, CL-20), industrial at gates along routes, rail or water, residential sorted by income band, parks on slopes and floodplain. A district maps onto the 100 to 500 m zone cells of [0038](../decisions/0038-grid-sector-zone-sizes-accepted.md); the zone record stores its land use.
4. **Streets per district.** A style from the culture and era kit: grid (tensor grid field aligned to the main arterial, CL-4), radial (centre element), organic (grid with rotation noise, or Watabou-like growth from gates), suburban (curved locals, cul-de-sacs, CL-39, CL-43). Spacing dsep comes from the target block size and intersection density of the style and region (CL-35, CL-36, CL-51). Two levels as in Chen (CL-7): arterials and collectors first, then locals per district.
5. **Blocks.** Faces of the planar street graph, minus street reserves by class (CL-41).
6. **Plots.** Vanegas subdivision per block (CL-9, CL-10): skeleton style for perimeter blocks and old towns, OBB style for suburbs; Amin, Amax, Wmin, Wmax per land use class and income; street access lambda = 1 for every plot that must hold an address.
7. **Assignment.** Households and businesses go to plots by class: one detached household per house plot, several per apartment plot (dwelling type from income and density, an open item in spec 51), businesses on commercial frontage. Plot count must equal the records plus a vacancy share; a shortfall subdivides or turns plots into apartments, a surplus adds vacant lots. Addresses come from the street edge and the side, replacing spec 51's numbering along abstract streets.

### Determinism and laziness

- Every step is a pure function of (world seed, town id, district id, block id) with keyed draws ([content tools](../design/content-tools.md) §1, rule 1). Steps 1 to 5 and the plot outlines run at town creation and are stored (they carry the addresses, so they are records); buildings and interiors stay on approach ([zoom continuum](../design/zoom-continuum.md) T3).
- Streamline tracing is sequential and float-sensitive (CL-8): snap every vertex to the 0.5 m build grid, use fixed-point or integer geometry for intersections, process seeds in a fixed key order, never in hash or pointer order ([determinism](determinism.md)).
- Plot subdivision is block-local (CL-13), so one block can be regenerated alone and must give the same bytes as in the full run.
- Cost: Vanegas' whole city in under 3 s and Evans' 5 to 10 s (CL-8, CL-13) suggest a town of a few thousand people, about 1,200 plots (CL-52), costs well under a second.

### Tests to write

| Test | Pass when | IDs |
| --- | --- | --- |
| Every plot reaches a street | Each plot shares an edge of length at least Wmin with a street reserve | CL-10 |
| Plots partition blocks | No overlaps; plot areas sum to block area within grid rounding | CL-11 |
| Network connected | One connected component; every gate reaches the centre; the country's route edges are in the graph | WNS-31 |
| Records match | Plots with homes equal households (or dwelling units equal households); every business has a plot; vacancy share in band; addresses unique | spec 51 |
| Land shares | Streets 20 to 35 % of built-up area; open space 10 to 20 %; each district within its land-use band | CL-23 to CL-26 |
| Network form | Intersections per km2, median segment length, dead-end share and circuity within the band of the style and region | CL-35, CL-36, CL-39 |
| Density | People per ha of built-up area within the band for country group, income and era | CL-30 to CL-32 |
| Lots | Plot area and frontage histograms within Amin, Amax and Wmin, Wmax of the class | CL-12 |
| Same seed, same bytes | Golden hash of a town's layout; one block regenerated alone equals the same block in the full run | CL-13 |

### Data to add

A `data/gen/urban_form.toml` with, per style and era: street reserve widths by class, target block size and intersection density, dead-end share, lot area and frontage per land-use class and income, density per country group, and the sources by finding ID. Culture packs point to a style.

## Open questions

1. Atlas of Urban Expansion 2016 volume 2 and "State of the Streets" (2018) hold the 1990-2015 numbers on road share, block size, four-way intersections and arterial access by region; not read here. They should set the bands for modern expansion areas.
2. Land use shares for whole cities (residential, commercial, industrial, green) by region were not found from a primary source; only UN-Habitat's floor mix for centres (CL-28).
3. Residential lot sizes for Latin America, Africa and South and East Asia, and lot sizes by income group, were not found.
4. Dwellings per ha by density class (detached, terraced, walk-up, tower) need a source table; only two examples (CL-44, CL-48).
5. Informal settlements: whether and how a generator for low-income countries draws unplanned subdivisions (narrow alleys, CL-41; densities over 1,000 per ha, CL-33).
6. Which style is the default per culture pack, and whether an old town core with a newer grid around it is one rule (era rings).
7. Whether city growth during play (zoning, [simcity](simcity.md)) reuses the same subdivision step on new blocks, so built and generated land look alike.
8. Example-based synthesis needs real parcel data; OpenStreetMap is ODbL and still needs a decision ([world systems](../design/world-systems.md) open list).

## Sources

- Parish, Y. and Mueller, P. (2001). Procedural modeling of cities. SIGGRAPH 2001. history.siggraph.org/?p=116923
- Chen, G., Esch, G., Wonka, P., Mueller, P. and Zhang, E. (2008). Interactive procedural street modeling. ACM TOG 27(3) 103. web.engr.oregonstate.edu/~zhange/images/street_sig08.pdf
- Vanegas, C., Kelly, T., Weber, B., Halatsch, J., Aliaga, D. and Mueller, P. (2012). Procedural generation of parcels in urban modeling. Computer Graphics Forum 31(2). cs.purdue.edu/cgvlab/papers/aliaga/eg2012.pdf
- Aliaga, D., Vanegas, C. and Benes, B. (2008). Interactive example-based urban layout synthesis. ACM TOG 27(5) 160. cs.purdue.edu/homes/bbenes/papers/Aliaga08ToG.pdf
- Aliaga et al. (2011). Urban layouts, SIGGRAPH course notes. cs.purdue.edu/cgvlab/urban/sg_2011_course/umc_SG11_02_urban_layouts.pdf
- Evans, M. (2015). Procedural Generation For Dummies: Roads. martindevans.me/game-development/2015/12/11/Procedural-Generation-For-Dummies-Roads/
- Kelly, G. and McCabe, H. (2006). A survey of procedural techniques for city generation. ITB Journal 14 (not read).
- Lechner, T., Watson, B., Wilensky, U. and Felsen, M. (2003). Procedural city modeling. ccl.northwestern.edu/testsite/rp/cities/papers/midgraph.cities.pdf (not read)
- SLEUTH: sciencebase.gov/catalog/item/5506f6dbe4b02e76d756e4d8; woodwellclimate.org/wp-content/uploads/2015/09/JantzetalJGIS.05.pdf (search summaries)
- Watabou. Medieval Fantasy City Generator. watabou.itch.io/medieval-fantasy-city-generator; github.com/watabou/TownGeneratorOS
- Space syntax. en.wikipedia.org/wiki/Space_syntax
- UN-Habitat (2014). A New Strategy of Sustainable Neighbourhood Planning: Five Principles. Urban Planning Discussion Note 3. unhabitat.org/sites/default/files/download-manager-files/A New Strategy of Sustainable Neighbourhood Planning Five principles.pdf
- Angel, S., Parent, J., Civco, D. and Blei, A. (2010). The Persistent Decline in Urban Densities: Global and Historical Evidence of Sprawl. Lincoln Institute working paper. lincolninst.edu/app/uploads/2024/04/1834_1085_angel_final_1.pdf
- Angel, S. et al. (2016). Atlas of Urban Expansion, 2016 edition, volumes 1 and 2. lincolninst.edu/publications/other/atlas-urban-expansion-2016-edition (not read)
- Lamson-Hall, P., Angel, S. and Liu, Y. (2018). The State of the Streets. Lincoln Institute working paper. lincolninst.edu/publications/working-papers/state-streets/ (not read)
- Boeing, G. (2020). Street network models and indicators for every urban area in the world. arXiv 2009.09106.
- Boeing, G. (2017). A multi-scale analysis of 27,000 urban street networks. arXiv 1705.02198.
- Wikipedia: Commissioners' Plan of 1811; Plan Cerdá (Spanish); Street hierarchy; Machiya; Public Land Survey System; Radburn, New Jersey; Floor area ratio; Laws of the Indies.
- US Census Bureau, Characteristics of New Housing, highlights. census.gov/construction/chars/highlights.html
