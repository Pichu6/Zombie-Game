"""Global tables and base-country demography from UN World Population Prospects 2024 (spec 43).

Reads the four WPP 2024 CSV files (CC BY 3.0 IGO) from a raw-data folder outside the repo, keeps the world and the
four World Bank income groups for one year, and writes:
  data/setup/world/wpp2024.toml                    the global tables (age and sex, life tables, fertility, indicators)
  data/setup/demography/wpp2024_<level>.toml       pop parameters fitted to them (mortality, fertility, birth sex ratio)
  data/setup/nations/base_<level>.toml             the base countries (age pyramid sourced; other tables placeholder)
and prints how well the fitted models reproduce the tables.

usage: python tools/data/wpp2024.py [RAW_DIR] [--year 2023]
"""
import csv
import gzip
import math
import pathlib
import sys

import numpy as np
from scipy.optimize import minimize

ROOT = pathlib.Path(__file__).resolve().parents[2]
DEFAULT_RAW = ROOT.parent / "data_raw" / "wpp2024"
SOURCE = "UN DESA Population Division, World Population Prospects 2024 (CC BY 3.0 IGO)"

# (level id, WPP location id, WPP name)
GROUPS = [
    ("world", 900, "World"),
    ("high", 1503, "High-income countries"),
    ("upper_middle", 1502, "Upper-middle-income countries"),
    ("lower_middle", 1501, "Lower-middle-income countries"),
    ("low", 1500, "Low-income countries"),
]
BASE_NAMES = {
    "high": "Base country, high income",
    "upper_middle": "Base country, upper-middle income",
    "lower_middle": "Base country, lower-middle income",
    "low": "Base country, low income",
}
FILES = {
    "pop": "WPP2024_PopulationByAge5GroupSex_Medium.csv.gz",
    "life": "WPP2024_Life_Table_Abridged_Medium_1950-2023.csv.gz",
    "fert": "WPP2024_Fertility_by_Age5.csv.gz",
    "ind": "WPP2024_Demographic_Indicators_Medium.csv.gz",
}
FIT_AGES = (15, 90)          # Gompertz-Makeham is fitted on the abridged groups starting in [15, 90)
FERT_MIN_AGE = 15.0          # the model's first possible birth (WPP's 10-14 band is under 1 % of births)
FERT_END_AGE = 50.0
FERT_GAP_MIN = 1.0           # the shortest interval between births, about nine months plus postpartum infertility
DT = 0.05                    # years, for the fertility renewal integral


def rows(raw, key, loc_ids, year):
    with gzip.open(raw / FILES[key], "rt", encoding="utf-8-sig", newline="") as f:
        for r in csv.DictReader(f):
            if r["Time"] == year and r["LocID"] and int(r["LocID"]) in loc_ids:
                yield r


def load(raw, year):
    ids = {g[1] for g in GROUPS}
    data = {g[1]: {"pop": [], "life": {1: [], 2: []}, "fert": [], "ind": None} for g in GROUPS}
    for r in rows(raw, "pop", ids, year):
        data[int(r["LocID"])]["pop"].append((int(r["AgeGrpStart"]), float(r["PopMale"]), float(r["PopFemale"])))
    for r in rows(raw, "life", ids, year):
        sex = int(r["SexID"])
        if sex in (1, 2):
            data[int(r["LocID"])]["life"][sex].append(
                (int(r["AgeGrpStart"]), int(r["AgeGrpSpan"]), float(r["mx"]), float(r["qx"]), float(r["ex"])))
    for r in rows(raw, "fert", ids, year):
        data[int(r["LocID"])]["fert"].append((int(r["AgeGrpStart"]), float(r["ASFR"]) / 1000.0))
    for r in rows(raw, "ind", ids, year):
        if r["Variant"] == "Medium":
            data[int(r["LocID"])]["ind"] = r
    for loc, d in data.items():
        if not d["pop"] or not d["life"][1] or not d["fert"] or d["ind"] is None:
            sys.exit(f"wpp2024: no {year} data for location {loc}")
        d["pop"].sort()
        d["fert"].sort()
        for s in (1, 2):
            d["life"][s].sort()
    return data


def bands20(pop):
    """Population shares in 20 five-year bands, 0-4 ... 90-94, 95+ (WPP's 95-99 and 100+ merged)."""
    male, female = [0.0] * 20, [0.0] * 20
    for start, m, f in pop:
        b = min(start // 5, 19)
        male[b] += m
        female[b] += f
    total = sum(male) + sum(female)
    return [(male[i] + female[i]) / total for i in range(20)], [m / total for m in male], [f / total for f in female]


# ---- Mortality: hazard A + B exp(C x), the model of zs::Mortality (spec 28).

def survival(a, b, c, x):
    return np.exp(-a * x - b / c * np.expm1(c * x))


def life_expectancy(a, b, c, from_age=0.0, to_age=130.0):
    x = np.arange(from_age, to_age, 0.01)
    s = survival(a, b, c, x) / survival(a, b, c, from_age)
    return float(np.trapezoid(s, x))


def fit_mortality(table):
    """Least squares on log mx at each group's midpoint, ages in FIT_AGES, weighted by group span."""
    pts = [(start + span / 2.0, mx, span) for start, span, mx, _, _ in table
           if FIT_AGES[0] <= start < FIT_AGES[1] and mx > 0]
    x = np.array([p[0] for p in pts])
    y = np.log(np.array([p[1] for p in pts]))
    w = np.array([p[2] for p in pts], dtype=float)

    def loss(theta):
        a, b, c = np.exp(theta)
        return float(np.sum(w * (np.log(a + b * np.exp(c * x)) - y) ** 2))

    best = None
    for c0 in (0.07, 0.09, 0.11):
        r = minimize(loss, np.log([5e-4, 2e-5, c0]), method="Nelder-Mead",
                     options={"xatol": 1e-10, "fatol": 1e-14, "maxiter": 20000, "maxfev": 40000})
        if best is None or r.fun < best.fun:
            best = r
    a, b, c = np.exp(best.x)
    return float(a), float(b), float(c)


# ---- Fertility: the age-specific model of zs::Fertility (spec 43). A woman's birth hazard is constant within each
# five-year band from 15 to 50; after a birth none for FERT_GAP_MIN years. Births per woman alive at age a:
# b(a) = h(a) * (1 - births in (a - gap, a]), since at most one birth fits in a gap. The hazards are fitted so the
# model's age-specific rates equal WPP's.

def birth_intensity(hazards):
    ages = np.arange(FERT_MIN_AGE, FERT_END_AGE, DT)
    h = np.array([hazards[int((a - FERT_MIN_AGE) // 5)] for a in ages])
    lag = int(round(FERT_GAP_MIN / DT))
    b = np.zeros_like(ages)
    for i in range(len(ages)):
        recent = b[max(0, i - lag):i].sum() * DT
        b[i] = h[i] * (1.0 - recent)
    return ages, b


def asfr_of(ages, b):
    return [float(np.mean(b[(ages >= s) & (ages < s + 5)])) for s in range(15, 50, 5)]


def fit_fertility(asfr):
    target = [v for s, v in asfr if 15 <= s < 50]
    hazards = list(target)
    for _ in range(200):
        model = asfr_of(*birth_intensity(hazards))
        hazards = [h * t / m if m > 0 else h for h, t, m in zip(hazards, target, model)]
    model = asfr_of(*birth_intensity(hazards))
    tfr = 5 * sum(model)
    mac = sum((15 + 5 * i + 2.5) * v for i, v in enumerate(model)) / sum(model)
    childless = math.exp(-5 * sum(hazards))  # no first birth: no dead time before it
    worst = max(abs(m - t) / t for m, t in zip(model, target) if t > 0)
    return hazards, tfr, mac, childless, worst


# ---- Output.

def fmt_list(xs, digits=6):
    return "[" + ", ".join(f"{x:.{digits}f}" for x in xs) + "]"


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    year = sys.argv[sys.argv.index("--year") + 1] if "--year" in sys.argv else "2023"
    if "--year" in sys.argv:
        args.remove(year)
    raw = pathlib.Path(args[0]) if args else DEFAULT_RAW
    data = load(raw, year)

    world = [f"# Global tables, {SOURCE}, data year {year}. Written by tools/data/wpp2024.py from the WPP CSV files;",
             "# do not edit by hand. Groups are the World Bank income groups as WPP aggregates them.", ""]
    report = []
    for level, loc, wpp_name in GROUPS:
        d = data[loc]
        ind = d["ind"]
        both, male, female = bands20(d["pop"])
        fits = {sex: fit_mortality(d["life"][sex]) for sex in (1, 2)}
        hazards, tfr_m, mac_m, childless, worst = fit_fertility(d["fert"])
        srb = float(ind["SRB"])
        e0 = {1: float(ind["LExMale"]), 2: float(ind["LExFemale"])}
        e15 = {s: next(ex for start, _, _, _, ex in d["life"][s] if start == 15) for s in (1, 2)}
        model_e0 = {s: life_expectancy(*fits[s]) for s in (1, 2)}
        model_e15 = {s: life_expectancy(*fits[s], from_age=15.0) for s in (1, 2)}
        report.append((level, e0, model_e0, e15, model_e15, float(ind["TFR"]), tfr_m, float(ind["MAC"]), mac_m, childless, worst))

        world += [f"[{level}]", f'wpp_location = "{wpp_name}"', f"wpp_loc_id = {loc}",
                  f"age_shares = {fmt_list(both)}  # 0-4 ... 90-94, 95+",
                  f"age_shares_male = {fmt_list(male)}", f"age_shares_female = {fmt_list(female)}"]
        for sex, name in ((1, "male"), (2, "female")):
            world.append(f"life_qx_{name} = {fmt_list([q for _, _, _, q, _ in d['life'][sex]])}  # abridged: 0, 1-4, 5-9 ... 100+")
        world += [f"asfr = {fmt_list([v for s, v in d['fert'] if 15 <= s < 50])}  # births per woman per year, 15-19 ... 45-49",
                  f"tfr = {float(ind['TFR']):.4f}", f"mean_age_childbearing = {float(ind['MAC']):.3f}",
                  f"sex_ratio_at_birth = {srb:.2f}  # boys per 100 girls",
                  f"life_expectancy_male = {e0[1]:.3f}", f"life_expectancy_female = {e0[2]:.3f}",
                  f"infant_mortality_per_1000 = {float(ind['IMR']):.3f}", f"under5_mortality_per_1000 = {float(ind['Q5']):.3f}",
                  f"median_age = {float(ind['MedianAgePop']):.3f}", ""]

        preset = [f"# Pop parameters fitted to {SOURCE}, {wpp_name}, {year}.",
                  "# Written by tools/data/wpp2024.py; do not edit by hand. Spec: docs/tier0/specs/43-global-tables.md",
                  f"# Fit: life expectancy at 15 model {model_e15[1]:.2f} / {model_e15[2]:.2f} (m/f) vs WPP {e15[1]:.2f} / {e15[2]:.2f};",
                  f"#      at birth model {model_e0[1]:.2f} / {model_e0[2]:.2f} vs WPP {e0[1]:.2f} / {e0[2]:.2f} (the model has no child-mortality term);",
                  f"#      TFR model {tfr_m:.3f} vs {float(ind['TFR']):.3f}; mean age at childbearing model {mac_m:.2f} vs {float(ind['MAC']):.2f}",
                  f"#      (WPP's own MAC is from single-year rates); worst band {100 * worst:.3f} %; never a mother {100 * childless:.1f} %.",
                  f'source = "{SOURCE}, {wpp_name}"', f"year = {year}", "",
                  f"male_share_at_birth = {srb / (100 + srb):.6f}",
                  "founder_min_age = 18.0", "founder_age_span = 47.0",
                  f"age_pyramid = {fmt_list(both)}", ""]
        for sex, name in ((2, "female"), (1, "male")):
            a, b, c = fits[sex]
            preset += [f"[mortality_{name}]  # Gompertz-Makeham on log mx, ages {FIT_AGES[0]}-{FIT_AGES[1] - 1}",
                       f"constant_hazard = {a:.6e}", f"gompertz_b = {b:.6e}", f"gompertz_c = {c:.6f}", ""]
        preset += ["[fertility]  # birth hazard by band 15-19 ... 45-49, fitted so the model's rates equal WPP's (spec 43)",
                   f"hazard_by_band = {fmt_list(hazards, 8)}", f"gap_min = {FERT_GAP_MIN}", "",
                   "[check]  # what the model gives, for the loader test (not read by the simulation)",
                   f"life_expectancy_15_male = {model_e15[1]:.4f}", f"life_expectancy_15_female = {model_e15[2]:.4f}", ""]
        (ROOT / "data/setup/demography" / f"wpp2024_{level}.toml").write_text("\n".join(preset), encoding="utf-8", newline="\n")

        if level in BASE_NAMES:
            write_base(level, both, year, wpp_name)

    out = ROOT / "data/setup/world"
    out.mkdir(parents=True, exist_ok=True)
    (out / "wpp2024.toml").write_text("\n".join(world), encoding="utf-8", newline="\n")

    print(f"{'group':14} {'e15 m/f model':>16} {'e15 m/f WPP':>14} {'e0 m/f model':>15} {'e0 m/f WPP':>13} {'TFR':>11} {'MAC':>11} {'childless':>9} {'worst':>7}")
    for level, e0, me0, e15, me15, tfr, tfr_m, mac, mac_m, childless, worst in report:
        print(f"{level:14} {me15[1]:7.2f}/{me15[2]:6.2f} {e15[1]:7.2f}/{e15[2]:6.2f} {me0[1]:7.2f}/{me0[2]:6.2f}"
              f" {e0[1]:6.2f}/{e0[2]:6.2f} {tfr_m:5.2f}/{tfr:4.2f} {mac_m:5.2f}/{mac:5.2f} {100 * childless:8.1f}% {100 * worst:6.3f}%")


def write_base(level, both, year, wpp_name):
    """A base country: the placeholder profile's other tables, with this group's sourced age pyramid and preset."""
    placeholder = (ROOT / "data/setup/nations/placeholder.toml").read_text(encoding="utf-8")
    body = placeholder.split("[education]", 1)[1]
    text = "\n".join([
        f"# {BASE_NAMES[level]}: a fictional country built from the {wpp_name} aggregate of {SOURCE}.",
        "# Written by tools/data/wpp2024.py. The age pyramid and the pop parameters (mortality, fertility, sex ratio at",
        "# birth) are sourced; education, labour, occupations, sectors, households and settlement are still the",
        "# placeholder shapes, so the profile's status stays \"placeholder\" until those tables are sourced too.",
        "",
        f'id = "base_{level}"',
        f'name = "{BASE_NAMES[level]}"',
        'status = "placeholder"',
        f'pop_params = "demography/wpp2024_{level}.toml"',
        "",
        "[age_pyramid]        # 20 five-year bands: 0-4, 5-9, ... 90-94, 95+",
        f"shares = {fmt_list(both)}",
        f'source = "{SOURCE}, {wpp_name}"',
        f"year = {year}",
        "",
        "[education]" + body,
    ])
    (ROOT / "data/setup/nations" / f"base_{level}.toml").write_text(text, encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
