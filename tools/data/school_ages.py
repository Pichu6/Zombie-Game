"""School stages by country from UNESCO UIS: the official entry age and length of primary, lower secondary and upper
secondary, and the years of compulsory education (research calendars-school-and-clock-hours, CK-209).

Input: data_raw/school/uis_ages_durations.json (UIS API indicators 299905 primary entry age, 299932 primary length,
999975 / 999976 lower secondary entry age / length, 999977 / 999978 upper secondary entry age / length, 400 compulsory
years; 2024-2025, the latest year per country).
Output: data/setup/school/ages.toml, one table per ISO3 and one per base country (from the same countries as the
calendars: USA, SRB, IND, ETH).
Usage: python -I tools/data/school_ages.py ../data_raw/school/uis_ages_durations.json data/setup/school/ages.toml
"""
import json
import sys
from pathlib import Path

FIELDS = {"299905": "primary_entry", "299932": "primary_years", "999975": "lower_entry", "999976": "lower_years",
          "999977": "upper_entry", "999978": "upper_years", "400": "compulsory_years"}
BASE_FROM = {"base_high": "USA", "base_upper_middle": "SRB", "base_lower_middle": "IND", "base_low": "ETH"}


def main(src, dst):
    recs = json.loads(Path(src).read_text(encoding="utf-8"))["records"]
    latest = {}
    for r in recs:
        key = (r["geoUnit"], FIELDS[r["indicatorId"]])
        if r["value"] is None:
            continue
        if key not in latest or r["year"] > latest[key][0]:
            latest[key] = (r["year"], int(r["value"]))
    countries = sorted({g for g, _ in latest})
    out = ["# School stages by country (spec 99 amendment). Written by tools/data/school_ages.py from UNESCO UIS (CC BY-SA 3.0 IGO);",
           "# do not edit by hand. Ages are the official entry ages; years the stages' lengths.", ""]
    rows = {}
    for g in countries:
        row = {f: latest[(g, f)][1] for f in FIELDS.values() if (g, f) in latest}
        if all(f in row for f in ("primary_entry", "primary_years", "lower_entry", "lower_years", "upper_entry", "upper_years")):
            rows[g] = row
    for base, iso in BASE_FROM.items():
        rows[base] = dict(rows[iso], **{"from": iso})
    for g in sorted(rows):
        out.append(f"[{g}]")
        for k, v in rows[g].items():
            out.append(f'{k} = "{v}"' if isinstance(v, str) else f"{k} = {v}")
        out.append("")
    Path(dst).parent.mkdir(parents=True, exist_ok=True)
    Path(dst).write_text("\n".join(out), encoding="utf-8", newline="\n")
    print(f"{len(rows)} tables written to {dst}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
