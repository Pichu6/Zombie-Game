"""Check the research notes in docs/research against decision 0054.

Checks: every note has the header (Date, Status, Feeds, Evidence, IDs) and a findings table
with the four confidence words; prefixes and IDs are unique; the README indexes every note once;
relative links resolve; a design doc a note feeds links back to it; a finding cited anywhere in
docs/ exists. Standard library only. Exit code 1 with one line per problem.

Usage: python3 tools/docs/check_research.py [repo root]
"""
import re
import sys
from pathlib import Path

CONFIDENCE = {"sourced", "reported", "derived", "estimate"}
HEADER_FIELDS = ("Date", "Status", "Feeds", "Evidence", "IDs")
LINK = re.compile(r"\[([^\]]*)\]\(([^)\s]+)\)")
ID_TEXT = re.compile(r"^([A-Z]{2,4})-(\d+)$")


def links(text):
    """Relative links in text, as (label, target without anchor)."""
    for label, target in LINK.findall(text):
        if re.match(r"^[a-z]+:", target) or target.startswith("#"):
            continue
        yield label, target.split("#", 1)[0]


def header_field(head, name):
    m = re.search(r"\*\*" + name + r":\*\*\s*(.*?)(?=\s*\*\*[A-Z][A-Za-z]*:\*\*|\n\n|$)", head, re.S)
    return m.group(1).strip() if m else None


def findings(text):
    """Rows of the table under '## Findings': list of (ID, confidence), or None if no section."""
    m = re.search(r"^## Findings\s*$(.*?)(?=^## |\Z)", text, re.M | re.S)
    if not m:
        return None
    rows = []
    for line in m.group(1).splitlines():
        cells = [c.strip() for c in line.strip().strip("|").split("|")] if line.startswith("|") else []
        if len(cells) >= 4 and ID_TEXT.match(cells[0]):
            rows.append((cells[0], cells[2].strip("*").lower()))
    if not rows and "None:" in m.group(1):
        return []
    return rows


def main(root):
    research = root / "docs" / "research"
    docs = root / "docs"
    errors = []
    notes = sorted(p for p in research.glob("*.md") if p.name != "README.md")
    prefixes = {}  # prefix -> note
    ids = {}  # note name -> set of IDs

    for note in notes:
        text = note.read_text(encoding="utf-8")
        name = note.name
        head = text.split("\n## ", 1)[0]
        for field in HEADER_FIELDS:
            if header_field(head, field) is None:
                errors.append(f"{name}: header has no **{field}:**")
        date = header_field(head, "Date") or ""
        if not re.match(r"\d{4}-\d{2}-\d{2}", date):
            errors.append(f"{name}: Date is not YYYY-MM-DD")
        prefix = (header_field(head, "IDs") or "").rstrip(".").strip("`")
        if not re.fullmatch(r"[A-Z]{2,4}", prefix):
            errors.append(f"{name}: IDs prefix '{prefix}' is not 2-4 capital letters")
        elif prefix in prefixes:
            errors.append(f"{name}: IDs prefix {prefix} already used by {prefixes[prefix]}")
        else:
            prefixes[prefix] = name
        feeds = header_field(head, "Feeds") or ""
        fed = [t for _, t in links(feeds) if t.startswith("../")]
        if "background" not in feeds.lower() and not fed:
            errors.append(f"{name}: Feeds links no design, spec or decision and does not say background")
        for target in fed:
            path = (research / target).resolve()
            if path.parent == (docs / "design").resolve() and path.exists():
                back = path.read_text(encoding="utf-8")
                if not any(Path(t).name == name for _, t in links(back)):
                    errors.append(f"{name}: feeds design/{path.name}, which does not link back")

        rows = findings(text)
        if rows is None:
            errors.append(f"{name}: no '## Findings' section")
            rows = []
        seen = set()
        for i, (fid, conf) in enumerate(rows, 1):
            m = ID_TEXT.match(fid)
            if m.group(1) != prefix:
                errors.append(f"{name}: finding {fid} does not use the prefix {prefix}")
            if fid in seen:
                errors.append(f"{name}: finding {fid} appears twice")
            seen.add(fid)
            if conf not in CONFIDENCE:
                errors.append(f"{name}: finding {fid} has confidence '{conf}', not one of {sorted(CONFIDENCE)}")
        ids[name] = seen

    # Index: every note linked exactly once from the README's index tables.
    readme = (research / "README.md").read_text(encoding="utf-8")
    m = re.search(r"^## Index\s*$(.*?)(?=^## |\Z)", readme, re.M | re.S)
    index = m.group(1) if m else ""
    counts = {}
    for _, t in links(index):
        if "/" not in t:
            counts[t] = counts.get(t, 0) + 1
    for note in notes:
        n = counts.get(note.name, 0)
        if n != 1:
            errors.append(f"README.md: {note.name} is indexed {n} times, not once")

    # Links resolve, and cited findings exist, across docs/.
    for md in sorted(docs.rglob("*.md")):
        text = md.read_text(encoding="utf-8")
        rel = md.relative_to(root)
        for label, target in links(text):
            if not target:
                continue
            path = (md.parent / target).resolve()
            if md.parent.resolve() == research.resolve():
                if not path.exists():
                    errors.append(f"{rel}: link to {target} does not resolve")
            m = ID_TEXT.match(label.strip())
            if m and path.parent == research.resolve() and path.name in ids:
                if label.strip() not in ids[path.name]:
                    errors.append(f"{rel}: cites {label} but {path.name} has no such finding")

    for e in errors:
        print(e)
    total = sum(len(s) for s in ids.values())
    print(f"{len(notes)} notes, {total} findings, {len(errors)} problems")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main(Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[2]))
