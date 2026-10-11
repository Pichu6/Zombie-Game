"""Country calendars (spec 53, decision 0072).

For every real-country profile (data/setup/nations/real/<ISO3>.toml) and every base country, writes
  data/setup/calendar/<id>.toml   weekend, main rest day, official short days, private week, public holidays
                                  (dated 2021-2031, fixed month-days, Easter offsets) and the school year
from raw files outside the repo (RAW_DIR, default ../data_raw), research note calendars-school-and-clock-hours (CK):
  calendar/gcal/<ISO3>.ics                              public holiday calendar feeds, 2021-2031 (CK-274, reported)
  derived-2026-10-11/ck-C-calendar-by-country.tsv       weekends, short days, working week (CK-247..CK-264)
  derived-2026-10-11/ck-B-school-by-country.tsv         school year: first month or day, instruction days, long break,
                                                        school week days (CK-205..CK-214)
Holidays are stored compactly: a list of unique names, and per year "MM-DD:i" codes (i indexes the names; a trailing
"h" marks a half day). Prints a summary.

usage: python tools/data/calendars.py [RAW_DIR]
"""
import csv
import datetime
import pathlib
import re
import statistics
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
DEFAULT_RAW = ROOT.parent / "data_raw"
YEARS = range(2021, 2032)
DAYS = ["mon", "tue", "wed", "thu", "fri", "sat", "sun"]

# Decision 0072 rule 5: the real country each base country copies.
BASE_FROM = {"base_high": "USA", "base_upper_middle": "SRB", "base_lower_middle": "IND", "base_low": "ETH"}
# Spec 53 rule 6: the main rest day where it is not Friday (weekend with Friday) or Sunday.
MAIN_REST_OVERRIDE = {"ISR": "sat", "NPL": "sat"}
# Spec 53 Data: UIS codes the US school year from July (CK-208); estimate.
SCHOOL_FIRST_DAY_OVERRIDE = {"USA": ("08-25", "estimate: US schools start in late August; UIS codes the year from July (CK-208)")}
# Federal countries whose feeds give most holidays per region: one large region stands for the country (CK Gaps).
REGION = {"AUS": "New South Wales", "BIH": "Federation of Bosnia and Herzegovina", "CAN": "Ontario", "CHE": "Zurich",
          "GBR": "England"}


def easter_western(y):
    """Anonymous Gregorian computus."""
    a, b, c = y % 19, y // 100, y % 100
    d, e = b // 4, b % 4
    f = (b + 8) // 25
    g = (b - f + 1) // 3
    h = (19 * a + b - d - g + 15) % 30
    i, k = c // 4, c % 4
    l_ = (32 + 2 * e + 2 * i - h - k) % 7
    m = (a + 11 * h + 22 * l_) // 451
    month = (h + l_ - 7 * m + 114) // 31
    day = (h + l_ - 7 * m + 114) % 31 + 1
    return datetime.date(y, month, day)


def easter_orthodox(y):
    """Julian computus (Meeus), converted to the Gregorian date (13 days apart in 1900-2099)."""
    a, b, c = y % 4, y % 7, y % 19
    d = (19 * c + 15) % 30
    e = (2 * a + 4 * b - d + 34) % 7
    month = (d + e + 114) // 31
    day = (d + e + 114) % 31 + 1
    return datetime.date(y, month, day) + datetime.timedelta(days=13)


def read_ics(path, region=None):
    """National public holidays from one feed: {date: (name, half)}; several names on one date are joined. With a
    region, that region's public holidays count too."""
    text = path.read_text(encoding="utf-8", errors="replace")
    text = re.sub(r"\r?\n[ \t]", "", text)  # unfold continuation lines
    out = {}
    for block in text.split("BEGIN:VEVENT")[1:]:
        start = re.search(r"DTSTART;VALUE=DATE:(\d{8})", block)
        end = re.search(r"DTEND;VALUE=DATE:(\d{8})", block)
        summary = re.search(r"\nSUMMARY:([^\r\n]*)", block)
        desc = re.search(r"\nDESCRIPTION:([^\r\n]*)", block)
        if not start:
            continue
        d = (desc.group(1) if desc else "").replace("\\,", ",")
        first = d.split("\\n")[0].strip()
        regional = region is not None and first.startswith("Public holiday in ") and region in first
        if first != "Public holiday" and not regional:  # observances and other regions' holidays are left out
            continue
        half = "half-day holiday" in d
        name = (summary.group(1) if summary else "").strip().replace("\\,", ",").replace("�", "'")
        name = re.sub(r"\s*\(tentative\)", "", name)
        day = datetime.datetime.strptime(start.group(1), "%Y%m%d").date()
        stop = datetime.datetime.strptime(end.group(1), "%Y%m%d").date() if end else day + datetime.timedelta(days=1)
        while day < stop:
            if day.year in YEARS:
                if day in out:
                    old_name, old_half = out[day]
                    if name not in old_name.split(" / "):
                        old_name = old_name + " / " + name
                    out[day] = (old_name, old_half and half)  # a full holiday on the same date wins
                else:
                    out[day] = (name, half)
            day += datetime.timedelta(days=1)
    return out


def rules(holidays):
    """Fixed month-days and Easter offsets present (as full or half holidays alike) in every year the feed covers.
    Some feeds stop early (governments that announce holidays year by year); their missing years use these rules."""
    by_year = {y: {d for d in holidays if d.year == y} for y in YEARS}
    by_year = {y: days for y, days in by_year.items() if days}
    fixed = None
    west, orth = None, None
    for y, days in by_year.items():
        md = {(d.month, d.day) for d in days}
        fixed = md if fixed is None else fixed & md
        w = {(d - easter_western(y)).days for d in days}
        o = {(d - easter_orthodox(y)).days for d in days}
        west = w if west is None else west & w
        orth = o if orth is None else orth & o
    west = {x for x in west if -60 <= x <= 70}
    orth = {x for x in orth if -60 <= x <= 70}
    # An offset that is also a fixed date every year cannot happen; an Easter date that coincides with a fixed date
    # in one year is fine. When both Easters give the same offsets (never in 2021-2031), keep the Western one.
    orth -= west if west == orth else set()
    return sorted(fixed or []), sorted(west), sorted(orth)


def name_at(holidays, pick):
    """The most common name among the dates a rule picks; a date with several names counts each of them."""
    counts = {}
    for d in sorted(holidays):
        if pick(d):
            for n in holidays[d][0].split(" / "):
                counts[n] = counts.get(n, 0) + 1
    return max(counts, key=lambda n: (counts[n], -len(n))) if counts else ""


def toml_str(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def weekend_days(text):
    text = text.strip()
    if not text:
        return ["sat", "sun"]
    return [t.strip().lower()[:3] for t in text.split("+")]


def school_first_day(row, iso3):
    if iso3 in SCHOOL_FIRST_DAY_OVERRIDE:
        return SCHOOL_FIRST_DAY_OVERRIDE[iso3]
    if row.get("eu_start_date_2023"):
        d = datetime.date.fromisoformat(row["eu_start_date_2023"])
        return f"{d.month:02d}-{d.day:02d}", "sourced: Eurydice 2023/24 first day (CK-210)"
    month = int(row["school_year_start_month"])
    return f"{month:02d}-01", f"{row['school_year_start_month_conf']}: first month from {row['school_year_start_month_src']} (CK-205); day 1 by rule"


def write_calendar(out_dir, cid, src_iso3, cal_row, school_row, holidays, base_note=""):
    weekend = weekend_days(cal_row["weekend_days"])
    main = MAIN_REST_OVERRIDE.get(src_iso3, "fri" if "fri" in weekend else ("sun" if "sun" in weekend else weekend[-1]))
    short = [d for d in weekend_days(cal_row["half_day"])] if cal_row["half_day"].strip() else []
    short = [d for d in short if d not in weekend]
    week = int(cal_row["workweek_days"]) if cal_row["workweek_days"].strip() else 5
    fixed, west, orth = rules(holidays)
    names = []

    def index(name):
        if name not in names:
            names.append(name)
        return names.index(name)

    lines = []
    lines.append(f"# Calendar of {cid} (spec 53, decision 0072). Written by tools/data/calendars.py; do not edit by hand.")
    if base_note:
        lines.append(f"# {base_note}")
    lines.append(f'id = "{cid}"')
    lines.append(f'from = "{src_iso3}"')
    lines.append("weekend = [" + ", ".join(toml_str(d) for d in weekend) + "]")
    lines.append(f'main_rest_day = "{main}"')
    lines.append("short_days = [" + ", ".join(toml_str(d) for d in short) + "]")
    lines.append(f"private_week_days = {week}")
    lines.append("source = " + toml_str(f"CK weekend table ({cal_row['source']}); research note calendars-school-and-clock-hours"))
    lines.append("confidence = " + toml_str(cal_row["confidence"] or "weekend=estimate"))
    lines.append("")
    hol = []
    hol.append("[holidays]")
    hol.append("source = " + toml_str(f"Google public holiday calendar feed for {src_iso3}, national public holidays 2021-2031 (CK-274); reported"))
    hol.append("fixed = [" + ", ".join(toml_str(f"{m:02d}-{d:02d}:{index(name_at(holidays, lambda x, m=m, d=d: (x.month, x.day) == (m, d)))}") for m, d in fixed) + "]")
    hol.append("easter_western = [" + ", ".join(toml_str(f"{o}:{index(name_at(holidays, lambda x, o=o: (x - easter_western(x.year)).days == o))}") for o in west) + "]")
    hol.append("easter_orthodox = [" + ", ".join(toml_str(f"{o}:{index(name_at(holidays, lambda x, o=o: (x - easter_orthodox(x.year)).days == o))}") for o in orth) + "]")
    dated = []
    for y in YEARS:
        if not any(x.year == y for x in holidays):
            continue  # the feed does not cover this year: spec 53 rule 7's rules fill it
        codes = []
        for d in sorted(x for x in holidays if x.year == y):
            name, half = holidays[d]
            codes.append(toml_str(f"{d.month:02d}-{d.day:02d}:{index(name)}" + ("h" if half else "")))
        dated.append(f"{y} = [" + ", ".join(codes) + "]")
    hol.append("names = [" + ", ".join(toml_str(n) for n in names) + "]")
    hol.append("")
    hol.append("[holidays.dates]")
    hol.extend(dated)
    hol.append("")
    first_day, first_note = school_first_day(school_row, src_iso3)
    sch = ["[school]"]
    sch.append(f'first_day = "{first_day}"')
    sch.append(f"long_break_weeks = {float(school_row['long_break_weeks']):.1f}")
    sch.append(f"instruction_days = {int(round(float(school_row['instruction_days_per_year'])))}")
    sch.append(f"week_days = {int(school_row['school_week_days'])}")
    sch.append("source = " + toml_str(
        f"first day: {first_note}; instruction days: {school_row['instruction_days_per_year_src']} "
        f"({school_row['instruction_days_per_year_conf']}); long break: {school_row['long_break_weeks_src']} "
        f"({school_row['long_break_weeks_conf']}); week days: {school_row['school_week_days_conf']} (CK school table)"))
    text = "\n".join(lines + hol + sch) + "\n"
    (out_dir / f"{cid}.toml").write_text(text, encoding="utf-8", newline="\n")
    return dict(weekend=weekend, main=main, holidays=holidays, fixed=fixed, west=west, orth=orth)


def main():
    raw = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_RAW
    derived = raw / "derived-2026-10-11"
    with open(derived / "ck-C-calendar-by-country.tsv", encoding="utf-8") as f:
        cal = {r["ISO3"]: r for r in csv.DictReader(f, delimiter="\t")}
    with open(derived / "ck-B-school-by-country.tsv", encoding="utf-8") as f:
        school = {r["iso3"]: r for r in csv.DictReader(f, delimiter="\t")}
    real = sorted(p.stem for p in (ROOT / "data/setup/nations/real").glob("*.toml"))
    out_dir = ROOT / "data/setup/calendar"
    out_dir.mkdir(parents=True, exist_ok=True)
    stats = {}
    missing = [c for c in real if c not in cal or c not in school or not (raw / "calendar/gcal" / f"{c}.ics").exists()]
    if missing:
        sys.exit(f"no calendar data for: {missing}")
    for c in real:
        hol = read_ics(raw / "calendar/gcal" / f"{c}.ics", REGION.get(c))
        stats[c] = write_calendar(out_dir, c, c, cal[c], school[c], hol)
    for base, src in BASE_FROM.items():
        hol = read_ics(raw / "calendar/gcal" / f"{src}.ics", REGION.get(src))
        write_calendar(out_dir, base, src, cal[src], school[src], hol,
                       base_note=f"The base country copies {src}'s calendar (decision 0072 rule 5).")
    per_2025 = [sum(1 for d, (n, h) in s["holidays"].items() if d.year == 2025 and not h) for s in stats.values()]
    empty = {c: [y for y in YEARS if not any(d.year == y for d in s["holidays"])] for c, s in stats.items()}
    empty = {c: f"{ys[0]}-{ys[-1]}" for c, ys in empty.items() if ys}
    no_rules = [c for c, s in stats.items() if c in empty and not s["fixed"]]
    print(f"{len(real)} countries + {len(BASE_FROM)} base countries written to {out_dir}")
    print(f"full holiday days in 2025: median {statistics.median(per_2025)}, min {min(per_2025)}, max {max(per_2025)}")
    print(f"feeds that stop early (rules fill those years): {empty}; of them without fixed rules: {no_rules}")
    print(f"fixed rules: median {statistics.median(len(s['fixed']) for s in stats.values())}; "
          f"Western Easter offsets in {sum(1 for s in stats.values() if s['west'])} countries, "
          f"Orthodox in {sum(1 for s in stats.values() if s['orth'])}")


if __name__ == "__main__":
    main()
