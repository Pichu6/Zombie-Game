"""Check the Try panel's manifest (data/atlas/tries.toml) against docs/status.md, and generate the status table's
"Try it" column from it (atlas design section 3 rule 3, spec 55 Open).

Checks: every manifest entry has an id, a title, a kind and a run; ids are unique; every `--flag` or `zs gen X` the
status table's Try column names is covered by some entry's run; every entry that names a spec finds a status row
that names that spec (a note, since the stage rows name none). With --write, the Try column of each status row that names a spec is replaced by the titles
and commands of the entries for that spec; rows without a spec keep their hand-written column.

Standard library only (the manifest is read by a small parser: the file uses only strings, arrays of strings, booleans
and dates). Exit code 1 with one line per problem.

Usage: python3 tools/docs/check_tries.py [repo root] [--write]
"""
import re
import sys
from pathlib import Path

ROW = re.compile(r"^\| (.*?) \| (.*?) \| (.*?) \|$")
SPEC_IN_ROW = re.compile(r"spec (\d+)")
CODE = re.compile(r"`([^`]+)`")


def parse_manifest(text):
    """[[try]] tables with key = value lines: strings, [strings], true/false, dates."""
    entries = []
    cur = None
    for raw in text.splitlines():
        line = raw.split("#", 1)[0].strip() if not raw.strip().startswith('"') else raw.strip()
        if not line:
            continue
        if line == "[[try]]":
            cur = {}
            entries.append(cur)
            continue
        if cur is None or "=" not in line:
            continue
        key, value = [p.strip() for p in line.split("=", 1)]
        if value.startswith("["):
            cur[key] = re.findall(r'"([^"]*)"', value)
        elif value.startswith('"'):
            cur[key] = value[1:-1]
        elif value in ("true", "false"):
            cur[key] = value == "true"
        else:
            cur[key] = value
    return entries


def command_of(entry):
    return "zs " + " ".join(entry.get("run", []))


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    write = "--write" in sys.argv
    root = Path(args[0]) if args else Path(__file__).resolve().parents[2]
    manifest = root / "data" / "atlas" / "tries.toml"
    status = root / "docs" / "status.md"
    problems = []
    entries = parse_manifest(manifest.read_text(encoding="utf-8"))
    ids = {}
    for e in entries:
        for key in ("id", "title", "kind", "run"):
            if key not in e:
                problems.append(f"tries.toml: an entry without {key}: {e}")
        if e.get("id") in ids:
            problems.append(f"tries.toml: id {e['id']} twice")
        ids[e.get("id")] = e
    by_spec = {}
    for e in entries:
        if e.get("spec"):
            by_spec.setdefault(e["spec"].lstrip("0"), []).append(e)

    lines = status.read_text(encoding="utf-8").split("\n")
    in_table = False
    specs_in_rows = set()
    out = []
    for line in lines:
        m = ROW.match(line)
        if line.startswith("| Part | What | Try it |"):
            in_table = True
            out.append(line)
            continue
        if in_table and (not m or line.startswith("| --- |")):
            if not m:
                in_table = False
            out.append(line)
            continue
        if not in_table:
            out.append(line)
            continue
        part, what, try_col = m.groups()
        spec = SPEC_IN_ROW.search(part)
        spec_no = spec.group(1).lstrip("0") if spec else None
        if spec_no:
            specs_in_rows.add(spec_no)
        # Every command the hand column names must be covered by some entry's run.
        for code in CODE.findall(try_col):
            if code in ("ctest", "zs_gen_tests", "zs_sim_tests") or code.startswith("data/"):
                continue
            tokens = code.split()
            covered = any(all(tok in e.get("run", []) for tok in tokens if tok.startswith("--") or tok in ("gen",) or (tokens[0] == "zs" and tok != "zs"))
                          for e in entries)
            flag_only = [t for t in tokens if t.startswith("--")]
            if flag_only and not any(all(f in e.get("run", []) for f in flag_only) for e in entries):
                problems.append(f"status.md row '{part[:40]}': `{code}` has no Try entry")
            elif not flag_only and tokens[0] == "zs" and not covered:
                problems.append(f"status.md row '{part[:40]}': `{code}` has no Try entry")
        if write and spec_no and spec_no in by_spec:
            cells = [f"{e['title']} (`{command_of(e)}`)" for e in by_spec[spec_no]]
            try_col = "; ".join(cells)
        out.append(f"| {part} | {what} | {try_col} |")
    # Stage rows ("Tier 0 stages 1-4") name no spec, so an entry without a matching row is a note, not a problem.
    for spec_no, es in sorted(by_spec.items(), key=lambda kv: int(kv[0])):
        if spec_no not in specs_in_rows:
            print(f"note: entries for spec {spec_no} ({', '.join(e['id'] for e in es)}) have no status row naming that spec; the hand column stays")
    if write:
        status.write_text("\n".join(out), encoding="utf-8", newline="\n")
        print(f"wrote the Try column for {len(specs_in_rows & set(by_spec))} rows")
    for p in problems:
        print(p)
    print(f"{len(entries)} entries, {len(problems)} problems")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
