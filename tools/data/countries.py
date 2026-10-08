"""Real-country profiles under parody names (decision 0053, spec 49).

For every country with UN WPP 2024 and ILO modelled data and a World Bank income group, writes
  data/setup/demography/real/<ISO3>.toml   pop parameters fitted to its life table and fertility (specs 43, 44)
  data/setup/nations/real/<ISO3>.toml      its profile: age pyramid, education, labour, occupations, sectors,
                                           households, settlement, named by data/setup/nations/parody_names.toml
from the same raw files as tools/data/global_tables.py. A table the country lacks falls back to its income group's
aggregate and says so in its source. Prints a summary and how well the fits reproduce each country.

usage: python tools/data/countries.py [RAW_DIR] [--year 2023] [--only ISO3,ISO3]
"""
import csv
import gzip
import json
import pathlib
import re
import sys

sys.path.insert(0, str(pathlib.Path(__file__).parent))
import global_tables as gt  # noqa: E402
import rds  # noqa: E402

ROOT = gt.ROOT
INCOME = {"HIC": "high", "UMC": "upper_middle", "LMC": "lower_middle", "LIC": "low"}


def mini_toml(text):
    """The subset of TOML our data files use: [sections], key = number | "string" | [numbers], # comments."""
    out, cur = {}, None
    for line in text.splitlines():
        line = re.sub(r'\s+#.*$', '', line).strip() if '"' not in line else line.strip()
        if not line or line.startswith("#"):
            continue
        m = re.match(r"^\[([^\]]+)\]", line)
        if m:
            cur = out.setdefault(m.group(1).strip(), {})
            continue
        k, v = (x.strip() for x in line.split("=", 1))
        v = re.sub(r"\s+#[^\"]*$", "", v)
        if v.startswith('"'):
            val = v[1:v.index('"', 1)]
        elif v.startswith("["):
            val = [float(t) for t in v.strip("[]").split(",") if t.strip()]
        else:
            val = float(v)
        (cur if cur is not None else out)[k] = val
    return out


def load_wpp(raw, year):
    """WPP rows for every country: population by age, life tables, fertility, indicators; by ISO3."""
    data = {}
    def d(iso):
        return data.setdefault(iso, {"pop": [], "life": {1: [], 2: []}, "fert": [], "ind": None, "loc": None})
    def each(key):
        with gzip.open(raw / "wpp2024" / gt.FILES[key], "rt", encoding="utf-8-sig", newline="") as f:
            for r in csv.DictReader(f):
                if r["Time"] == year and r["LocTypeName"] == "Country/Area" and r["ISO3_code"]:
                    yield r
    for r in each("pop"):
        d(r["ISO3_code"])["pop"].append((int(r["AgeGrpStart"]), float(r["PopMale"]), float(r["PopFemale"])))
    for r in each("life"):
        if int(r["SexID"]) in (1, 2):
            d(r["ISO3_code"])["life"][int(r["SexID"])].append(
                (int(r["AgeGrpStart"]), int(r["AgeGrpSpan"]), float(r["mx"]), float(r["qx"]), float(r["ex"])))
    for r in each("fert"):
        d(r["ISO3_code"])["fert"].append((int(r["AgeGrpStart"]), float(r["ASFR"]) / 1000.0))
    for r in each("ind"):
        if r["Variant"] == "Medium":
            x = d(r["ISO3_code"])
            x["ind"], x["loc"] = r, int(r["LocID"])
    for x in data.values():
        x["pop"].sort()
        x["fert"].sort()
        for s in (1, 2):
            x["life"][s].sort()
    return data


def ilo_by_area(raw, year):
    part, unemp, occ, sec = {}, {}, {}, {}
    for r in gt.ilo_rows(raw, "DF_EAP_2WAP_SEX_AGE_RT", year):
        part[(r["REF_AREA"], r["SEX"], r["AGE"])] = float(r["OBS_VALUE"]) / 100
    for r in gt.ilo_rows(raw, "DF_UNE_2EAP_SEX_AGE_RT", year):
        if r["SEX"] == "SEX_T" and r["AGE"] == "AGE_YTHADULT_YGE15":
            unemp[r["REF_AREA"]] = float(r["OBS_VALUE"]) / 100
    for r in gt.ilo_rows(raw, "DF_EMP_2EMP_SEX_OCU_NB", year):
        if r["SEX"] == "SEX_T":
            occ[(r["REF_AREA"], r["OCU"])] = float(r["OBS_VALUE"])
    for r in gt.ilo_rows(raw, "DF_EMP_2EMP_SEX_ECO_NB", year):
        if r["SEX"] == "SEX_T" and r["ECO"].startswith("ECO_SECTOR_"):
            sec[(r["REF_AREA"], r["ECO"])] = float(r["OBS_VALUE"])
    return part, unemp, occ, sec


def labour_of(area, part, unemp):
    try:
        groups = [[part[(area, sex, "AGE_AGGREGATE_" + age)] for sex in ("SEX_M", "SEX_F")]
                  for age in ("Y15-24", "Y25-54", "Y55-64", "YGE65")]
        return {"participation": groups, "unemployment": unemp[area]}
    except KeyError:
        return None


def shares_of(values):
    total = sum(values)
    return [v / total for v in values] if total > 0 else None


def education_by_country(raw, year):
    d = rds.read_rds(raw / "wittgenstein" / "pop-age-edattain.rds")
    level_of = {"No Education": 0, "Incomplete Primary": 0, "Primary": 1, "Lower Secondary": 2, "Upper Secondary": 2,
                "Post Secondary": 3}
    w = (int(year) - 2020) / 5.0
    sums = {}
    for code, y, a, e, p in zip(d["country_code"], d["year"], d["age"], d["education"], d["pop"]):
        if y not in (2020.0, 2025.0) or e not in level_of or code is None:
            continue
        lo = int(a.split("--")[0].rstrip("+"))
        if lo < 25:
            continue
        g = 0 if lo < 35 else 1 if lo < 65 else 2
        rows = sums.setdefault(int(code), [[0.0] * 4 for _ in range(3)])
        rows[g][level_of[e]] += ((1 - w) if y == 2020.0 else w) * p
    return {loc: [[x / sum(r) for x in r] for r in rows] for loc, rows in sums.items() if all(sum(r) > 0 for r in rows)}


def households_by_country(raw):
    latest = {}
    for r in gt.xlsx_rows(raw / "undesa_households" / "undesa_pd_2022_hh-size-composition.xlsx", 3):
        try:
            loc, size = int(r.get("B") or ""), float(r.get("E") or "")
            groups = [float(r[c]) for c in "FGHI"]
        except (TypeError, ValueError, KeyError):
            continue
        year = gt.ref_year(r.get("D"))
        if year is None or not 95 <= sum(groups) <= 105:
            continue
        if loc not in latest or year > latest[loc][0]:
            latest[loc] = (year, size, [g / sum(groups) for g in groups])
    return latest


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    year = sys.argv[sys.argv.index("--year") + 1] if "--year" in sys.argv else "2023"
    only = set(sys.argv[sys.argv.index("--only") + 1].split(",")) if "--only" in sys.argv else None
    for flag in ("--year", "--only"):
        if flag in sys.argv:
            args.remove(sys.argv[sys.argv.index(flag) + 1])
    raw = pathlib.Path(args[0]) if args else gt.DEFAULT_RAW

    names = mini_toml((ROOT / "data/setup/nations/parody_names.toml").read_text(encoding="utf-8"))["names"]
    income = {k: INCOME[v] for k, v in gt.load_income(raw).items() if v in INCOME}
    wpp = load_wpp(raw, year)
    part, unemp, occ, sec = ilo_by_area(raw, year)
    education = education_by_country(raw, year)
    households = households_by_country(raw)
    _, urban_rows = json.loads((raw / "ilostat" / "wb_SP.URB.TOTL.IN.ZS_all.json").read_text(encoding="utf-8"))
    urban = {r["countryiso3code"]: r["value"] / 100 for r in urban_rows if r["value"] is not None and r["countryiso3code"]}
    group_nation = {lv: mini_toml((ROOT / f"data/setup/nations/base_{lv}.toml").read_text(encoding="utf-8"))
                    for lv in INCOME.values()}

    out_n = ROOT / "data/setup/nations/real"
    out_d = ROOT / "data/setup/demography/real"
    out_n.mkdir(parents=True, exist_ok=True)
    out_d.mkdir(parents=True, exist_ok=True)
    written, fallbacks = [], {}
    for iso in sorted(names):
        if only and iso not in only:
            continue
        x = wpp.get(iso)
        if iso not in income or x is None or x["ind"] is None or not x["life"][1] or not x["fert"]:
            continue
        lv = income[iso]
        real = x["ind"]["Location"]
        both, _, _ = gt.bands20(x["pop"])
        fits = {s: gt.fit_mortality(x["life"][s]) for s in (1, 2)}
        hazards, tfr_m, mac_m, childless, worst = gt.fit_fertility(x["fert"])
        e0 = {1: float(x["ind"]["LExMale"]), 2: float(x["ind"]["LExFemale"])}
        me0 = {s: gt.life_expectancy(fits[s]) for s in (1, 2)}
        me15 = {s: gt.life_expectancy(fits[s], from_age=15.0) for s in (1, 2)}
        srb = float(x["ind"]["SRB"])
        src = f"{gt.SOURCE}, {real}"
        preset = [f"# Pop parameters fitted to {src}, {year}. Written by tools/data/countries.py; do not edit by hand.",
                  f"# Fit: e0 model {me0[1]:.2f}/{me0[2]:.2f} vs WPP {e0[1]:.2f}/{e0[2]:.2f}; TFR model {tfr_m:.3f} vs {float(x['ind']['TFR']):.3f}.",
                  f'source = "{src}"', f"year = {year}", "",
                  f"male_share_at_birth = {srb / (100 + srb):.6f}", "founder_min_age = 18.0", "founder_age_span = 47.0",
                  f"age_pyramid = {gt.fmt_list(both)}", ""]
        for sex, nm in ((2, "female"), (1, "male")):
            a, b, c, dd, ee = fits[sex]
            preset += [f"[mortality_{nm}]", f"constant_hazard = {a:.6e}", f"gompertz_b = {b:.6e}", f"gompertz_c = {c:.6f}",
                       f"child_hazard = {dd:.6e}", f"child_decline = {ee:.6f}", ""]
        preset += ["[fertility]", f"hazard_by_band = {gt.fmt_list(hazards, 8)}", f"gap_min = {gt.FERT_GAP_MIN}", "",
                   "[check]", f"life_expectancy_0_male = {me0[1]:.4f}", f"life_expectancy_0_female = {me0[2]:.4f}",
                   f"life_expectancy_15_male = {me15[1]:.4f}", f"life_expectancy_15_female = {me15[2]:.4f}", ""]
        (out_d / f"{iso}.toml").write_text("\n".join(preset), encoding="utf-8", newline="\n")

        missing = []
        g = group_nation[lv]
        gname = gt.WB_AGGREGATE_NAME[lv]
        def fallback(table):
            missing.append(table)
            return f"{gname} group aggregate (no {table} data for {real})"
        edu = education.get(x["loc"])
        edu_src = f'{gt.EDU_SOURCE}, {real}' if edu else fallback("education")
        if not edu:
            edu = [g["education"][k] for k in ("age_25_34", "age_35_64", "age_65_plus")]
        lab = labour_of(iso, part, unemp)
        lab_src = f"{gt.ILO_SOURCE}, {real}; retirement_age is an estimate, not data" if lab else fallback("labour")
        if not lab:
            gl = g["labour"]
            lab = {"participation": [gl[k] for k in ("participation_15_24", "participation_25_54", "participation_55_64", "participation_65_plus")],
                   "unemployment": gl["unemployment_rate"]}
        oc = shares_of([occ.get((iso, o), 0.0) for o in gt.OCCUPATIONS])
        oc_src = f"{gt.ILO_SOURCE}, {real}" if oc else fallback("occupation")
        oc = oc or g["occupations"]["shares"]
        se = shares_of([sec.get((iso, e), 0.0) for e in ("ECO_SECTOR_AGR", "ECO_SECTOR_IND", "ECO_SECTOR_SER")])
        se_src = f"{gt.ILO_SOURCE}, {real}" if se else fallback("sector")
        se = se or g["sectors"]["shares"]
        hh = households.get(x["loc"])
        if hh:
            hh_sizes, hh_mean, hh_year = hh[2], hh[1], hh[0]
            hh_src = f"{gt.HH_SOURCE}, {real}, latest survey"
        else:
            hh_sizes, hh_mean, hh_year = g["households"]["sizes"], g["households"]["mean_size"], g["households"]["year"]
            hh_src = fallback("household")
        ur = urban.get(iso)
        ur_src = f"{gt.URBAN_SOURCE}, {real}" if ur is not None else fallback("urbanisation")
        ur = ur if ur is not None else g["settlement"]["urban_share"]
        fallbacks[iso] = missing
        people = float(x["ind"]["TPopulation1July"]) * 1000

        text = "\n".join([
            f"# {names[iso]}: a country with the real data of {real} under a parody name (decision 0053).",
            "# Written by tools/data/countries.py; do not edit by hand.", "",
            f'id = "{iso}"', f'name = "{names[iso]}"', 'status = "sourced"', f'pop_params = "demography/real/{iso}.toml"', "",
            "[age_pyramid]", f"shares = {gt.fmt_list(both)}", f'source = "{src}"', f"year = {year}", "",
            "[education]", f"age_25_34 = {gt.fmt_list(edu[0])}", f"age_35_64 = {gt.fmt_list(edu[1])}",
            f"age_65_plus = {gt.fmt_list(edu[2])}", f'source = "{edu_src}"', f"year = {year}", "",
            "[labour]", f"participation_15_24 = {gt.fmt_list(lab['participation'][0], 4)}",
            f"participation_25_54 = {gt.fmt_list(lab['participation'][1], 4)}",
            f"participation_55_64 = {gt.fmt_list(lab['participation'][2], 4)}",
            f"participation_65_plus = {gt.fmt_list(lab['participation'][3], 4)}",
            f"unemployment_rate = {lab['unemployment']:.4f}", f"retirement_age = {gt.RETIREMENT_AGE}",
            f'source = "{lab_src}"', f"year = {year}", "",
            "[occupations]", f"shares = {gt.fmt_list(oc)}", f'source = "{oc_src}"', f"year = {year}", "",
            "[sectors]", f"shares = {gt.fmt_list(se)}", f'source = "{se_src}"', f"year = {year}", "",
            "[households]", f"sizes = {gt.fmt_list(hh_sizes)}", f"mean_size = {hh_mean:.3f}", f'source = "{hh_src}"',
            f"year = {hh_year}", "",
            "[settlement]", f"urban_share = {ur:.4f}", f'source = "{ur_src}"', f"year = {year}", "",
            "[country]  # read by the world generator (spec 49), not by the profile loader's tables",
            f'income = "{lv}"', f"population = {people:.0f}", "",
        ])
        (out_n / f"{iso}.toml").write_text(text, encoding="utf-8", newline="\n")
        written.append((iso, names[iso], lv, people, me0, e0, tfr_m, float(x["ind"]["TFR"])))
        print(f"{iso} {names[iso]:28} {lv:13} e0 {me0[1]:5.1f}/{me0[2]:5.1f} (WPP {e0[1]:5.1f}/{e0[2]:5.1f})"
              f" TFR {tfr_m:4.2f} ({float(x['ind']['TFR']):4.2f}) fallbacks: {', '.join(missing) or '-'}", flush=True)
    worst = max((max(abs(m[s] - e[s]) for s in (1, 2)), iso) for iso, _, _, _, m, e, _, _ in written)
    print(f"{len(written)} countries; worst e0 gap {worst[0]:.2f} years ({worst[1]}); "
          f"{sum(1 for v in fallbacks.values() if v)} with some table from their group")


if __name__ == "__main__":
    main()
