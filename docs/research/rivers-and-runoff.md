# Rivers, runoff and wood stock: numbers for the land generator

**Date:** 2026-10-09. **Status:** research note, not a decision. **Feeds:** [spec 52](../tier0/specs/52-land-water-capacity.md), [world systems](../design/world-systems.md), [0059](../decisions/0059-countries-from-water-land-and-resources-everything-alive-can-be-sick.md). **Evidence:** web-search summaries of the abstracts and dataset pages named below; the full papers and the FRA 2020 report tables were not read. **IDs:** RV.

The generator needs rivers that carry a believable share of the rain, a rule that splits rain into evaporation and runoff by climate, and a wood stock per hectare of forest. These are the numbers it is checked against.

## Findings

| ID | Finding | Confidence | Source |
| --- | --- | --- | --- |
| RV-1 | Rivers carry about 37,288 ± 662 km³ per year from the continents to the oceans (1.2 Sv), about 35 % of the precipitation on land and 7.6 % of global precipitation, from 921 gauged rivers plus estimates for unmonitored areas. | reported | Dai and Trenberth 2002, https://data.ucar.edu/dataset/estimates-of-freshwater-discharge-from-continents-latitudinal-and-seasonal-variations |
| RV-2 | Precipitation on land is therefore about 106,500 km³ per year. | derived | RV-1: 37,288 / 0.35 |
| RV-3 | Fu's form of the Budyko curve splits precipitation P into actual evaporation E by the aridity PET/P: E/P = 1 + PET/P − (1 + (PET/P)^ω)^(1/ω); ω = 2.6 is a common default, fitted per basin in practice and linked to vegetation and seasonality. | reported | https://hess.copernicus.org/articles/21/1515/2017/ ; https://hess.copernicus.org/articles/20/393/2016/ |
| RV-4 | The Thornthwaite method estimates potential evaporation from temperature and day length alone and is used at global scale for its small input needs, but it is weak where wind, humidity and radiation dominate. | reported | https://essd.copernicus.org/articles/14/163/2022/ |
| RV-5 | Annual potential evaporation from the annual mean temperature: PET = max(100, 40 T + 200) mm (T in °C). Not a published formula: a single-number stand-in for Thornthwaite, calibrated on the default seed-1 planet (frequency 24) so that its runoff ratio is 34 % against RV-1's 35 % (a sweep of 30-50 mm per °C and 150-300 mm gave 27-42 %). | estimate | this note |
| RV-6 | The world's forest growing stock is 557 billion m³ (FAO, 2020) on 4.06 billion ha of forest. | reported | https://www.fao.org/forestry-fao/industries/98951/en |
| RV-7 | The mean growing stock is about 137 m³ per hectare of forest. | derived | RV-6: 557e9 / 4.06e9 |
| RV-8 | Shares of forest area by climatic domain: tropical 45 %, boreal 27 %, temperate 16 %, subtropical 11 %. | reported | https://en.wikipedia.org/wiki/Global_Forest_Resources_Assessment |
| RV-9 | A river class for the map: a cell is a river when at least 100 m³/s and a catchment of 100,000 km² pass through it, a large, navigable river at 1,000 m³/s and 500,000 km² (for scale: the Rhine drains about 185,000 km² and carries about 2,300 m³/s at its mouth, the Danube about 800,000 km², the Mississippi about 3.2 million). The catchment keeps the classes independent of the grid's cell size. Thresholds and the scale figures are not from a read source. | estimate | this note |

## How the generator uses them

Runoff per land cell is P − E with E from Fu (RV-3) and PET from RV-5; water flows to the lowest neighbour after depressions are filled, and discharge is the sum of the runoff upstream. The test checks the planet's runoff ratio against RV-1 and the size of its largest river against the Amazon's order of magnitude. Wood stock per hectare of forest (slice 2 of spec 52) starts from RV-7, scaled by biome.

## Open questions

- A sourced temperature-to-PET relation (Thornthwaite's monthly form needs the months, which the world fields do not keep) or a global PET map (CGIAR Global Aridity Index) to replace RV-5.
- Growing stock per hectare by climatic domain (the FRA 2020 tables were not read).
