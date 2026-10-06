# World numbers: demography, structure, education, labour, inequality, production

**Date:** 2026-10-06. **Status: research note, not a decision.** Feeds [0030](../decisions/0030-nation-demographics.md) and [0031](../decisions/0031-realistic-world-numbers-and-the-pop-nation-loop.md); nothing here is *Decided*, *Implemented* or *Validated*.

## Method and limits (read first)

- WebFetch was **blocked** by the network egress proxy (ourworldindata.org, wikipedia.org, rug.nl all returned EGRESS_BLOCKED). **Every number below comes from WebSearch result summaries**, not from reading the source page. Treat confidence as at most medium; re-check against the dataset itself before any `sourced` profile ([0030](../decisions/0030-nation-demographics.md) refuses `sourced` without source and year).
- A number with no URL is not in the Numbers table. Quantities I know from general economics but could not confirm by search are listed under "Unverified / not found", not presented as fact.
- Where a search summary looked internally inconsistent it is flagged, not used.

## Numbers

| Quantity | Value or range | Unit | Source URL | Confidence |
| --- | --- | --- | --- | --- |
| Global total fertility rate, 1950 | 4.84 | births per woman | https://www.ncbi.nlm.nih.gov/pmc/articles/PMC11122687/ | medium (GBD estimate) |
| Global total fertility rate, 2021 | 2.23 | births per woman | https://www.ncbi.nlm.nih.gov/pmc/articles/PMC11122687/ | medium |
| Global life expectancy, c. 1800 | about 30 | years at birth | https://ourworldindata.org/life-expectancy | medium |
| No world region above 40 years of life expectancy in 1800 | < 40 | years | https://ourworldindata.org/life-expectancy | medium |
| Global life expectancy, 2019 / 2021 | 72.6 (2019); just over 70 (2021) | years | https://ourworldindata.org/life-expectancy | medium |
| Speed of fertility decline, early transitions (Bangladesh, Brazil, Indonesia) | about 1 child per decade | TFR per 10 years | https://www.un.org/en/development/desa/population/events/pdf/expert/4/bongaarts.pdf | low (summary of search snippet) |
| Fast fertility transitions (Iran, China) | from above 6 to below 3 in 10 to 11 years | years | https://www.un.org/development/desa/pd/sites/www.un.org.development.desa.pd/files/unpd_egm_200203_backgroundpaper_fertility_levels_and_trends_population_division.pdf | low |
| Later transitions are faster than earlier ones | qualitative | n/a | https://www.niussp.org/fertility-and-reproduction/the-pace-of-fertility-decline-in-sub-saharan-africa/ | low |
| World median age, 2024 | 31.6 | years | https://www.un.org/development/desa/pd/node/4324 | medium |
| Median age Africa / Europe, 2024 | 20.2 / 43.5 | years | https://www.un.org/development/desa/pd/node/4324 | medium |
| Children under 15 per 100 working-age people, Africa / Europe, 2024 | 68.3 / 23.8 | ratio x100 | https://www.un.org/development/desa/pd/node/4324 | medium |
| Urban share of population, high-income countries, 2020 | 81.76 | % | https://www.indexmundi.com/facts/high-income/urban-population | medium (World Bank series via mirror) |
| Urban share, low-income countries, 1990 and 2012 | 21 and 28 | % | https://www.indexmundi.com/facts/low-income/urban-population | low (snippet) |
| Agriculture share of employment, DR Congo vs USA (recent) | 69 vs 1 | % of employed | https://blogs.worldbank.org/en/opendata/incomes-rise-farm-employment-drops | medium |
| Agriculture employment falls as income rises (modelled ILO estimates, since 1991) | qualitative, strong negative | n/a | https://blogs.worldbank.org/en/opendata/incomes-rise-farm-employment-drops | medium |
| Long-run sector employment shares (1801-2015) exist for 9+ countries | dataset, not extracted | % | https://archive.ourworldindata.org/20260518-090244/grapher/share-of-agriculture-in-total-employment.html | n/a (not read) |
| Private return to one year of schooling, world average | 9 | % wage per year | https://documents1.worldbank.org/curated/zh/442521523465644318/pdf/WPS8402.pdf | medium-high (1120 estimates, 139 countries, 1950-2014) |
| That 9% is stable across decades; private returns to higher education rose; social returns above 10% at secondary and higher | qualitative | % | https://documents1.worldbank.org/curated/zh/442521523465644318/pdf/WPS8402.pdf | medium |
| Penn World Table human-capital Mincer returns, piecewise | 13.4 (years 0-4), 10.1 (4-8), 6.8 (8+) | % per year | https://cran.r-project.org/web/packages/pwt10/refman/pwt10.html | medium-high |
| PWT human capital input | average years of schooling, age 25+, from Barro-Lee, Cohen-Soto | years | https://cran.r-project.org/web/packages/pwt10/refman/pwt10.html | medium |
| Barro-Lee coverage | 146 countries, 5-year steps 1950-2010 (later extended to 2015), by sex, 7 schooling levels, ages 15+ and 25+ | n/a | https://splash-db.eu/dataresource/barro-lee-educational-attainment-data/ | medium |
| Labour force participation, world, 2023 | men about 72, women about 47 | % of 15+ | https://migration.ucdavis.edu/rmn/blog/post/?id=2856 | medium |
| Global unemployment rate, 2023 | 5.1 | % of labour force | https://migration.ucdavis.edu/rmn/blog/post/?id=2856 | medium |
| Average Gini, 126 World Bank countries | 40.8 | Gini x100 | https://ourworldindata.org/grapher/gini-coefficient-world-bank-marimekko | low-medium (snippet; mixes income and consumption surveys) |
| Global top 10% share of income / bottom 50% | 52 / 8.5 | % | https://wir2022.wid.world/www-site/uploads/2022/03/0098-21_WIL_RIM_EXECUTIVE_SUMMARY.pdf | medium |
| Global top 10% share of wealth / bottom 50% | 76 / 2 | % | https://wir2022.wid.world/www-site/uploads/2022/03/0098-21_WIL_RIM_EXECUTIVE_SUMMARY.pdf | medium |
| Wealth Gini is roughly twice the income Gini | about 2x | ratio | https://arxiv.org/pdf/1406.7115 | low (snippet from a secondary summary) |
| Top-1% shares and Gini rose in most countries since 1970, most in US then UK | qualitative | n/a | https://repec.cepr.org/repec/cpr/ceprdp/DP8071.pdf | low-medium |
| Intergenerational income elasticity (father-son), US | 0.4 to 0.6 plausible | elasticity | https://www.nber.org/papers/w34144 | medium |
| Great Gatsby curve: higher Gini goes with higher persistence | positive slope | n/a | https://www.nber.org/papers/w34144 | medium |
| Agglomeration elasticity of earnings to city size (Duranton-Puga) | 0.04 short run, 0.08 long run | elasticity | https://arxiv.org/pdf/2404.13178 | low-medium (snippet) |
| Doubling density raises productivity | 2 to 4 | % | https://arxiv.org/pdf/2404.13178 | low-medium |
| Urban cost elasticity to city size | about 0.03 (100k people) to 0.08 (Paris size) | elasticity | https://arxiv.org/pdf/2404.13178 | low |
| Gravity-model migration, elasticity to distance | -2 to -1, decaying with distance | elasticity | https://ideas.repec.org/a/eee/ecolet/v204y2021ics0165176521001506.html | low (snippet) |
| Preston curve: concave, shifts up over time (life expectancy rises at fixed income) | qualitative | n/a | https://hdr.undp.org/content/has-preston-curve-broken-down | medium |
| Maddison: world GDP per head rose 1271% over 1820-2020 | about 13.7x | ratio | https://www.rug.nl/ggdc/blog/the-maddison-project-new-2023-update-illuminates-origins-of-modern-economic-growth | medium |
| Maddison 2023 preindustrial coverage | about 2800 data points | n/a | https://www.rug.nl/ggdc/blog/the-maddison-project-new-2023-update-illuminates-origins-of-modern-economic-growth | medium |
| Malthusian regime end (Galor) | about 1750 developed, about 1900 less developed | year | https://www.nber.org/papers/w6811 | low-medium (theory summary) |
| Labour share fell globally since the early 1980s; investment-goods price decline explains about half | qualitative / 50 | % of decline | https://www.nber.org/papers/w19136.pdf | medium |
| TFP growth is better described as linear than exponential (Britain 1600-1914, US 1890-) | qualitative | n/a | https://www.noahpinion.blog/p/contemplating-a-world-of-linear-growth?sd=pf | low (blog summary of a claim) |

Rejected: a search summary gave a Preston slope of "4.125 years per unit of log income, 0.731 years per doubling" (https://metricgate.com/blogs/preston-curve-plateau-that-is-not-there/). These two do not agree (4.125 x ln 2 is about 2.9), so neither is used.

## Equations we could implement

All are **Proposals**. "Realistic band" is where real data are expected to sit; "sandbox band" is a wide range for what-if worlds. **Where no source is cited in the Numbers table, the parameter is a design placeholder to be fitted to a licensed dataset, not a real-world fact.** Let `y` = GDP per head relative to a reference (log scale), `t` = era (years since the nation's takeoff), `a` = age.

1. **Life expectancy (Preston form).** `e0 = e_min + (e_max(t) - e_min) * (1 - exp(-k * ln(y / y0)))` or simply `e0 = a(t) + b * ln(y)`. Shape sourced (concave, curve shifts up with era, [UNDP](https://hdr.undp.org/content/has-preston-curve-broken-down)); anchors from the table: about 30 years in 1800 (no region above 40), about 72 in 2019. `a(t)` rises with era independent of income. Fit `b` to WPP + Maddison/PWT. Realistic band: fit; sandbox: `e0` 25 to 90.
2. **Mortality by age.** Use a model life table family indexed by `e0` (a Gompertz-Makeham hazard `h(a) = A + B * exp(c * a)` with infant term; `A` high at low `e0`). Mortality schedule parameters: not found by search; fit to WPP life tables.
3. **Fertility transition.** `TFR(t) = TFR_low + (TFR_high - TFR_low) / (1 + exp(s * (y_ln - y_ln_mid)))`, plus an era speed-up term. Anchors: global 4.84 (1950) to 2.23 (2021); speed about 1 TFR per decade for early cases, 6 to under 3 in 10 to 11 years for fast cases (low confidence). Realistic band: `TFR_high` 5 to 7, `TFR_low` 1.2 to 2.1 (ranges are design values, not sourced); sandbox: 0.8 to 9.
4. **Age structure.** Stable-population projection: cohort-component step each year, births = TFR x age-specific fertility shape x women; survival from (2). Output: age pyramid, median age (check against 20.2 Africa, 31.6 world, 43.5 Europe in 2024), dependency ratio (68.3 and 23.8 children per 100 working-age as checks).
5. **Sector shares (structural change).** Logistic or log-linear in `ln y`: `s_agri = 1 / (1 + exp(g_a * (ln y - m_a)))`, `s_ind` hump-shaped (quadratic in `ln y`), `s_serv = 1 - s_agri - s_ind`. Anchors: DR Congo 69% vs US 1% agriculture. Midpoints and slopes: fit to the ILO / GGDC 10-sector data; not extracted here.
6. **Occupation groups (ISCO).** Given sector shares and education mix, assign ISCO major groups by a multinomial logit on (education level, sector). Group shares by income: **not found**; fit to ILOSTAT ISCO tables.
7. **Education attainment by cohort.** Mean years of schooling `S(c, y)` for birth cohort `c`: logistic in income at age of schooling, with a cohort-time shift (Barro-Lee shows steady cohort gains). Pop's years: draw from a distribution around `S` with sd about 0.3 to 0.5 of the mean (design placeholder). Enrolment and literacy: not extracted (see not found).
8. **Wages and human capital (Mincer).** `ln w = ln w0 + sum_j r_j * s_j` over schooling bands `j`, with `r = 0.134, 0.101, 0.068` (the PWT piecewise returns, [source](https://cran.r-project.org/web/packages/pwt10/refman/pwt10.html)) or the flat world mean of 0.09 ([Psacharopoulos and Patrinos](https://documents1.worldbank.org/curated/zh/442521523465644318/pdf/WPS8402.pdf)). Add experience: `+ b1 * x + b2 * x^2` (coefficients not sourced). Human capital per worker `h = exp(sum_j r_j * s_j)`, and effective labour `L_eff = sum over workers of h_i`.
9. **Labour force.** `LFP(a, sex, y, t) = base_sex(a) * hump(y)`: men near 72% and women near 47% of 15+ globally (2023); female rate U-shaped in income (the U-shape is a known literature result, **not confirmed by search**). Unemployment around 5% globally; urban and youth higher; retirement age and pension rules as nation policy variables.
10. **Production (human-capital-augmented Cobb-Douglas).** `Y = A * K^alpha * (h * L)^(1 - alpha)` with `alpha` about 1/3 as the classic calibration (the PWT labour-share series is the data to fit; the labour share has fallen since the 1980s, [NBER](https://www.nber.org/papers/w19136.pdf)). Output per worker `y_w = Y / L = A * k^alpha * h^(1-alpha)`, `k = (K/L)` adjusted. Sandbox band: `alpha` 0.2 to 0.6.
11. **TFP by era.** `dlnA/dt = g(t)` with `g` near 0 in a Malthusian era (population absorbs output; income per head flat, [Galor-Weil](https://www.nber.org/papers/w6811)), rising after takeoff; Maddison gives world GDP per head x13.7 over 1820-2020, about 1.3% per year compound (computed here: 13.71^(1/200) - 1, my arithmetic from the cited 1271%). Per-era TFP growth rates by country: **not found**; fit to PWT `ctfp`.
12. **Urban premium.** `A_city = A_nation * (pop_city / pop_ref)^eta`, `eta` 0.04 (short run) to 0.08 (long run) ([source](https://arxiv.org/pdf/2404.13178)); costs rise with elasticity about 0.03 to 0.08, so net benefit is a policy trade-off (the Cities: Skylines lever).
13. **Migration (gravity).** `M_ij = G * P_i^a * P_j^b * f(w_j / w_i) / d_ij^g`, `g` in 1 to 2 (distance elasticity -2 to -1, [source](https://ideas.repec.org/a/eee/ecolet/v204y2021ics0165176521001506.html)); rural-urban as Harris-Todaro: move if `p * w_urban > w_rural`, where `p` = urban employment probability ([source](https://en.wikipedia.org/wiki/Harris%E2%80%93Todaro_model), from search summary).
14. **Income distribution.** Draw pop income `ln y_i = mu + sigma * z_i`, lognormal with `sigma = sqrt(2) * Phi^-1((G + 1) / 2)` (the closed-form lognormal Gini relation, a standard identity). Gini by income level and era is not extracted; global mean 40.8; wealth Gini about 2x income Gini. Add a Pareto tail for the top. Intergenerational: `ln y_child = rho * ln y_parent + e`, `rho` (IGE) 0.4 to 0.6 in the US, positively tied to the Gini (Great Gatsby curve).
15. **Health spending feedback.** `delta_e0 = b_h * ln(1 + health_spend)`; coefficient not found (search only gave a positive, non-linear link and the US as outlier, [source](https://www.ineteconomics.org/perspectives/blog/the-link-between-health-spending-and-life-expectancy-the-us-is-an-outlier)).

## How this closes the loop with pops and nations (max 8 bullets)

- Nation income `y` and era `t` set life expectancy, fertility and schooling; those set the pop age pyramid and education mix next year.
- The pop table (age x sex x education x sector x wealth) sets effective labour `L_eff = sum h_i` and the dependency ratio; the nation's output is `A * K^alpha * L_eff^(1-alpha)`.
- Output per worker feeds back as `y`: richer nations get longer life, lower fertility, more schooling, and move workers out of agriculture.
- Schooling is a nation policy (spend, years): it raises `h` with a lag of one generation (Mincer returns), so education investment pays off in decades, not turns.
- Sector shift is driven by `ln y`; the worker pool re-sorts into occupations, which moves wages and the Gini.
- Urbanisation and migration move pops to cities with an urban premium `eta`, offset by costs (housing, health), giving the city-builder trade-off.
- Inequality sets mobility (Gatsby curve): a high-Gini nation has persistent classes, shaping unrest and tax base.
- Malthusian regime for early eras: output per head stuck, population absorbs gains; takeoff needs TFP growth and a fertility decline.

## Unverified / not found

- **Not found by search** (no cited number): crude birth and death rates by income group and era; fertility and life expectancy as a function of income (only shapes); age-specific mortality parameters; sector employment shares by GDP per head (only two country points and the qualitative law); ISCO major-group shares by income level; school enrolment and literacy history (literacy 1820 / 1900 / 2020 not returned); education attainment by income; labour force participation by age and by income; unemployment ranges by income; Gini by income level and era (only a global mean); top-1% shares by country and era; intergenerational elasticities outside the US; capital share value from PWT; TFP growth rates by era; health spending versus life expectancy coefficients; gravity-model population exponents; rural-to-urban flow rates.
- **Known from general economics but not confirmed here, so not in the table:** the classic Clark and Kuznets three-sector pattern values; capital share near 0.3 to 0.4 (a textbook calibration); female LFP U-shape; typical early-20th century literacy rates. Do not present these as sourced.
- The Preston slope figure from one blog was internally inconsistent and was dropped.
- Several rows rest on a snippet of a secondary page (Wikipedia, blogs, mirrors); confidence flagged "low".
- WebFetch was blocked, so no primary page was read.

## Data licences

Licences below come from search summaries unless marked. We only use public-domain or CC BY data. Check each dataset's own licence page before shipping any table ([0030](../decisions/0030-nation-demographics.md) notes).

| Dataset | Licence found | Use here | Source |
| --- | --- | --- | --- |
| UN World Population Prospects 2024 | CC BY 3.0 IGO (attribution required) | ok, with attribution | https://www.un.org/development/desa/pd/node/4327 |
| Maddison Project Database 2023 | CC BY 4.0 | ok | https://www.rug.nl/ggdc/blog/the-maddison-project-new-2023-update-illuminates-origins-of-modern-economic-growth |
| ILOSTAT (labour force statistics) | CC BY 4.0 (as shown on a mirror, DBnomics) | ok, confirm on ilostat.ilo.org | https://db.nomics.world/ILO/EAP_2WAP_SEX_AGE_RT |
| Penn World Table 10 | not found in search; to confirm on the GGDC site | do not use until confirmed | https://cran.r-project.org/web/packages/pwt10/refman/pwt10.html |
| Barro-Lee educational attainment | not found | do not use until confirmed | https://splash-db.eu/dataresource/barro-lee-educational-attainment-data/ |
| World Bank Open Data (WDI) | not found in search; to confirm on the World Bank terms page | do not use until confirmed | https://blogs.worldbank.org/en/opendata/incomes-rise-farm-employment-drops |
| Our World in Data charts | not found in search; to confirm (its grapher pages state a licence per chart) | do not use until confirmed | https://ourworldindata.org/grapher/economic-inequality-gini-index |
| World Inequality Database / Report | not found | do not use until confirmed | https://wir2022.wid.world/www-site/uploads/2022/03/0098-21_WIL_RIM_EXECUTIVE_SUMMARY.pdf |
| Global Burden of Disease fertility estimates | not found (IHME terms are usually non-commercial; confirm) | do not use | https://ghdx.healthdata.org/print/548961 |
| UBS Global Wealth Report | proprietary, not CC BY | do not ship its tables | https://www.ubs.com/jp/ja/about/newsdisplay-ndp/en-20260630-gwr-2026.html |
